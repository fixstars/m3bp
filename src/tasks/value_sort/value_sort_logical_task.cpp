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
#include <algorithm>
#include <cstring>
#include "tasks/value_sort/value_sort_logical_task.hpp"
#include "tasks/physical_task_command_base.hpp"
#include "context/execution_context.hpp"
#include "scheduler/locality_option.hpp"
#include "memory/serialized_buffer.hpp"

namespace m3bp {

class ValueSortLogicalTask::ValueSortCommand
	: public PhysicalTaskCommandBase
{
private:
	ValueSortLogicalTask *m_logical_task;
	MemoryReference m_source;
	LockedMemoryReference m_locked_source;
	identifier_type m_partition;
public:
	ValueSortCommand(
		ValueSortLogicalTask *logical_task,
		MemoryReference source_buffer,
		identifier_type partition)
		: m_logical_task(logical_task)
		, m_source(std::move(source_buffer))
		, m_locked_source()
		, m_partition(partition)
	{ }
	virtual void prepare(
		ExecutionContext & /* context */,
		const Locality & /* locality */) override
	{
		m_locked_source = m_source.lock();
		m_source = MemoryReference();
	}
	virtual void run(
		ExecutionContext &context,
		const Locality & /* locality */) override
	{
		m_logical_task->run(
			context, std::move(m_locked_source), m_partition);
	}
};


ValueSortLogicalTask::ValueSortLogicalTask()
	: LogicalTaskBase()
	, m_comparator()
{ }

ValueSortLogicalTask::ValueSortLogicalTask(ComparatorType comparator)
	: LogicalTaskBase()
	, m_comparator(std::move(comparator))
{ }


void ValueSortLogicalTask::create_physical_tasks(ExecutionContext &context){
	auto &scheduler = context.scheduler();
	const auto entry_id = scheduler.create_physical_task(
		task_id(),
		std::unique_ptr<PhysicalTaskCommandBase>(
			new PhysicalTaskCommandBase()),
		LocalityOption());
	const auto terminal_id = scheduler.create_physical_task(
		task_id(),
		std::unique_ptr<PhysicalTaskCommandBase>(
			new PhysicalTaskCommandBase()),
		LocalityOption());
	scheduler.add_dependency(entry_id, terminal_id);
	entry_task(entry_id);
	barrier_task(entry_id);
	terminal_task(terminal_id);
}

void ValueSortLogicalTask::commit_physical_tasks(ExecutionContext &context){
	auto &scheduler = context.scheduler();
	scheduler.commit_task(entry_task());
	scheduler.commit_task(terminal_task());
}

void ValueSortLogicalTask::receive_fragment(
	ExecutionContext &context,
	identifier_type port,
	identifier_type partition,
	MemoryReference mobj)
{
	(void)(port);
	assert(port == 0);
	auto &scheduler = context.scheduler();
	auto &locality_manager = context.locality_manager();
	const auto mobj_loc = mobj.locality();
	const auto pid = scheduler.create_physical_task(
		task_id(),
		std::unique_ptr<PhysicalTaskCommandBase>(
			new ValueSortCommand(this, std::move(mobj), partition)),
		LocalityOption(
			locality_manager.random_worker_from_node(mobj_loc)));
	scheduler.add_dependency(entry_task(), pid);
	scheduler.add_dependency(pid, terminal_task());
	scheduler.commit_task(pid);
}

void ValueSortLogicalTask::run(
	ExecutionContext &context,
	LockedMemoryReference mobj,
	identifier_type partition)
{
	using PairType = std::pair<const void *, identifier_type>;
	auto &memory_manager = context.memory_manager();
	SerializedBuffer input_sb(std::move(mobj));
	const auto value_count = input_sb.record_count();
	const auto group_count = input_sb.group_count();
	const auto keys_size = input_sb.keys_data_size();
	const auto values_size = input_sb.values_data_size();

	auto output_sb = SerializedBuffer::allocate_grouped_buffer(
		memory_manager, value_count, group_count, keys_size, values_size);
	memcpy(output_sb.keys_data(), input_sb.keys_data(), keys_size);
	memcpy(
		output_sb.keys_offsets().data(), input_sb.keys_offsets().data(),
		(group_count + 1) * sizeof(size_type));
	memcpy(
		output_sb.value_group_offsets().data(),
		input_sb.value_group_offsets().data(),
		(group_count + 1) * sizeof(size_type));

	const auto in_values =
		static_cast<const uint8_t *>(input_sb.values_data());
	const auto in_group_offsets = input_sb.value_group_offsets();
	const auto in_value_offsets = input_sb.values_offsets();
	const auto out_values = static_cast<uint8_t *>(output_sb.values_data());
	auto out_value_offsets = output_sb.values_offsets();
	out_value_offsets[0] = 0;
	for(m3bp::identifier_type g = 0, v = 0; g < group_count; ++g){
		const auto values_head = v;
		std::vector<PairType> ptr_pairs;
		while(in_value_offsets[v] < in_group_offsets[g + 1]){
			ptr_pairs.emplace_back(in_values + in_value_offsets[v], v);
			++v;
		}
		std::sort(
			ptr_pairs.begin(), ptr_pairs.end(),
			[this](const PairType &a, const PairType &b) -> bool {
				return m_comparator(a.first, b.first);
			});
		auto w = values_head;
		for(const auto &p : ptr_pairs){
			const auto i = p.second;
			const auto len = in_value_offsets[i + 1] - in_value_offsets[i];
			memcpy(out_values + out_value_offsets[w], p.first, len);
			out_value_offsets[w + 1] = out_value_offsets[w] + len;
			++w;
		}
	}

	commit_fragment(
		context, 0, partition, MemoryReference(output_sb.raw_reference()));
}

}

