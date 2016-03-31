/*
 * Copyright 2016 Fixstars Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <vector>
#include <cassert>
#include "tasks/gather/gather_logical_task.hpp"
#include "tasks/physical_task_command_base.hpp"
#include "context/execution_context.hpp"
#include "memory/serialized_buffer.hpp"
#include "scheduler/locality_option.hpp"

namespace m3bp {

namespace {

class GatherPhysicalCommand : public PhysicalTaskCommandBase {

private:
	GatherLogicalTask *m_logical_task;
	std::vector<LockedMemoryReference> m_locked_inputs;

public:
	explicit GatherPhysicalCommand(GatherLogicalTask *logical_task)
		: m_logical_task(logical_task)
		, m_locked_inputs()
	{ }

	virtual void prepare(
		ExecutionContext & /* context */,
		const Locality & /* locality */) override
	{
		assert(m_logical_task);
		auto unlocked = m_logical_task->inputs();
		std::vector<LockedMemoryReference> locked(unlocked.size());
		auto unlocked_it = unlocked.begin();
		auto locked_it = locked.begin();
		while(unlocked_it != unlocked.end()){
			*(locked_it++) = (unlocked_it++)->lock();
		}
		m_locked_inputs = std::move(locked);
	}

	virtual void run(
		ExecutionContext &context,
		const Locality & /* locality */) override
	{
		m_logical_task->run_gather(context, std::move(m_locked_inputs));
	}

};


size_type compute_actual_values_size(const SerializedBuffer &sb){
	const auto offsets = sb.values_offsets();
	return offsets[sb.record_count()] - offsets[0];
}

void append_serialized_buffer(
	SerializedBuffer &dst, const SerializedBuffer &src)
{
	const auto base_count = dst.record_count();
	const auto src_count = src.record_count();
	const auto src_offsets = src.values_offsets();
	auto dst_offsets = dst.values_offsets();
	dst_offsets[0] = 0;
	for(size_type i = 1; i <= src_count; ++i){
		dst_offsets[base_count + i] =
			dst_offsets[base_count] + src_offsets[i] - src_offsets[0];
	}
	const auto src_data =
		reinterpret_cast<const uint8_t *>(src.values_data());
	const auto dst_data = reinterpret_cast<uint8_t *>(dst.values_data());
	memcpy(
		dst_data + dst_offsets[base_count],
		src_data + src_offsets[0],
		src_offsets[src_count] - src_offsets[0]);
	dst.record_count(base_count + src.record_count());
}

}


GatherLogicalTask::GatherLogicalTask()
	: m3bp::LogicalTaskBase()
	, m_mutex()
	, m_inputs()
{ }


void GatherLogicalTask::create_physical_tasks(ExecutionContext &context){
	auto &scheduler = context.scheduler();
	const auto logical_task_id = task_id();
	const auto physical_task_id = scheduler.create_physical_task(
		logical_task_id,
		std::unique_ptr<PhysicalTaskCommandBase>(
			new GatherPhysicalCommand(this)),
		LocalityOption());
	entry_task   (physical_task_id);
	barrier_task (physical_task_id);
	terminal_task(physical_task_id);
}

void GatherLogicalTask::commit_physical_tasks(ExecutionContext &context){
	auto &scheduler = context.scheduler();
	scheduler.commit_task(entry_task());
}


ArrayRef<MemoryReference> GatherLogicalTask::inputs(){
	const auto ptr = m_inputs.data();
	return ArrayRef<MemoryReference>(ptr, ptr + m_inputs.size());
}

void GatherLogicalTask::run_gather(
	ExecutionContext &context,
	std::vector<LockedMemoryReference> inputs)
{
	assert(inputs.size() == m_inputs.size());
	auto &memory_manager = context.memory_manager();
	// extract metadata of each buffer
	const auto source_count = inputs.size();
	std::vector<SerializedBuffer> sources(source_count);
	for(identifier_type i = 0; i < source_count; ++i){
		sources[i] = SerializedBuffer(std::move(inputs[i]));
	}
	// compute the size of the destination buffer
	size_type total_record_size = 0;
	size_type total_record_count = 0;
	for(const auto &src : sources){
		total_record_count += src.record_count();
		total_record_size += compute_actual_values_size(src);
	}
	// allocate the destination buffer
	auto dst_sb = SerializedBuffer::allocate_value_only_buffer(
		memory_manager, total_record_count, total_record_size);
	// copy from source to destination
	for(const auto &src_sb : sources){
		append_serialized_buffer(dst_sb, src_sb);
	}
	dst_sb.record_count(total_record_count);
	// commit and release buffers
	commit_fragment(context, 0, 0, MemoryReference(dst_sb.raw_reference()));
	m_inputs.clear();
}


void GatherLogicalTask::receive_fragment(
	ExecutionContext & /* context */,
	identifier_type port,
	identifier_type partition,
	MemoryReference mobj)
{
	(void)(port);
	assert(port == 0);
	(void)(partition);
	assert(partition == 0);
	std::lock_guard<std::mutex> lock(m_mutex);
	m_inputs.emplace_back(std::move(mobj));
}

}

