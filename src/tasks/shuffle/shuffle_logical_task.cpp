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
#include <cassert>
#include <cstring>
#include "tasks/shuffle/shuffle_logical_task.hpp"
#include "tasks/shuffle/msd_radix_sort.hpp"
#include "tasks/shuffle/shuffle_buffer.hpp"
#include "tasks/physical_task_command_base.hpp"
#include "common/hash_function.hpp"
#include "context/execution_context.hpp"
#include "scheduler/locality.hpp"
#include "scheduler/locality_option.hpp"
#include "memory/serialized_buffer.hpp"

namespace m3bp {

namespace {

class ShufflePartitionCommand : public PhysicalTaskCommandBase {
private:
	ShuffleLogicalTask *m_logical_task;
	MemoryReference m_unlocked_source;
	LockedMemoryReference m_locked_source;
public:
	ShufflePartitionCommand(
		ShuffleLogicalTask *logical_task,
		MemoryReference source_buffer)
		: m_logical_task(logical_task)
		, m_unlocked_source(std::move(source_buffer))
		, m_locked_source()
	{ }
	virtual void prepare(
		ExecutionContext & /* context */,
		const Locality & /* locality */) override
	{
		m_locked_source = m_unlocked_source.lock();
		m_unlocked_source = MemoryReference();
	}
	virtual void run(
		ExecutionContext &context,
		const Locality &locality) override
	{
		m_logical_task->partition_fragment(
			context, locality, std::move(m_locked_source));
	}
};

class ShuffleSortCommand : public PhysicalTaskCommandBase {
private:
	ShuffleLogicalTask *m_logical_task;
	identifier_type m_partition;
	std::vector<MemoryReference> m_unlocked_sources;
	std::vector<LockedMemoryReference> m_locked_sources;
public:
	ShuffleSortCommand(
		ShuffleLogicalTask *logical_task,
		identifier_type partition,
		std::vector<MemoryReference> sources)
		: m_logical_task(logical_task)
		, m_partition(partition)
		, m_unlocked_sources(std::move(sources))
		, m_locked_sources()
	{ }
	virtual void prepare(
		ExecutionContext & /* context */,
		const Locality & /* locality */) override
	{
		std::vector<LockedMemoryReference> locked(m_unlocked_sources.size());
		auto unlocked_it = m_unlocked_sources.begin();
		auto locked_it = locked.begin();
		while(unlocked_it != m_unlocked_sources.end()){
			*(locked_it++) = (unlocked_it++)->lock();
		}
		m_locked_sources = std::move(locked);
		m_unlocked_sources.clear();
	}
	virtual void run(
		ExecutionContext &context,
		const Locality & /* locality */) override
	{
		m_logical_task->sort_records(
			context, std::move(m_locked_sources), m_partition);
	}
};

class ShuffleBarrierCommand : public PhysicalTaskCommandBase {
private:
	ShuffleLogicalTask *m_logical_task;
public:
	explicit ShuffleBarrierCommand(
		ShuffleLogicalTask *logical_task)
		: m_logical_task(logical_task)
	{ }
	virtual void run(
		ExecutionContext &context,
		const Locality & /* locality */) override
	{
		m_logical_task->create_sort_tasks(context);
	}
};

}


ShuffleLogicalTask::ShuffleLogicalTask(size_type partition_count)
	: LogicalTaskBase()
	, m_mutex()
	, m_partition_count(partition_count)
	, m_partitioned_buffers()
{ }

void ShuffleLogicalTask::create_physical_tasks(ExecutionContext &context){
	auto &scheduler = context.scheduler();
	const auto entry_id = scheduler.create_physical_task(
		task_id(),
		std::unique_ptr<PhysicalTaskCommandBase>(
			new PhysicalTaskCommandBase()),
		LocalityOption());
	const auto barrier_id = scheduler.create_physical_task(
		task_id(),
		std::unique_ptr<PhysicalTaskCommandBase>(
			new ShuffleBarrierCommand(this)),
		LocalityOption());
	const auto terminal_id = scheduler.create_physical_task(
		task_id(),
		std::unique_ptr<PhysicalTaskCommandBase>(
			new PhysicalTaskCommandBase()),
		LocalityOption());
	scheduler
		.add_dependency(entry_id, barrier_id)
		.add_dependency(barrier_id, terminal_id);
	entry_task(entry_id);
	barrier_task(barrier_id);
	terminal_task(terminal_id);
}

void ShuffleLogicalTask::commit_physical_tasks(ExecutionContext &context){
	auto &scheduler = context.scheduler();
	scheduler.commit_task(entry_task());
	scheduler.commit_task(barrier_task());
	scheduler.commit_task(terminal_task());
}


void ShuffleLogicalTask::partition_fragment(
	ExecutionContext &context,
	const Locality &locality,
	LockedMemoryReference mobj)
{
	auto &memory_manager = context.memory_manager();
	const SerializedBuffer src_sb(std::move(mobj));
	const auto in_data = static_cast<const uint8_t *>(src_sb.values_data());
	const auto in_offsets = src_sb.values_offsets();
	const auto in_key_lengths = src_sb.key_lengths();
	const size_type in_record_count = src_sb.record_count();
	const size_type partition_count = m_partition_count;

	std::vector<size_type> record_counts(partition_count);
	std::vector<size_type> size_sums(partition_count);
	std::vector<unsigned int> partitions(in_record_count);
	for(identifier_type i = 0; i < in_record_count; ++i){
		const unsigned int p =
			static_cast<unsigned int>(hash_byte_sequence(
				in_data + in_offsets[i], in_key_lengths[i], partition_count));
		record_counts[p] += 1;
		size_sums[p] += in_offsets[i + 1] - in_offsets[i];
		partitions[i] = p;
	}

	// record format:
	//   size_type record_length
	//   size_type key_length
	//   byte[]    key+value
	const auto total_buffer_size =
		(in_offsets[in_record_count] - in_offsets[0]) +
		2 * sizeof(size_type) * in_record_count;
	ShuffleBuffer dst_sb(
		memory_manager, total_buffer_size, partition_count,
		locality.self_node_id());
	auto dst_offsets = dst_sb.offsets();
	dst_offsets[0] = 0;
	std::vector<size_type> cur_offsets(partition_count);
	for(identifier_type i = 0; i < partition_count; ++i){
		dst_offsets[i + 1] =
			dst_offsets[i] + size_sums[i] +
			2 * sizeof(size_type) * record_counts[i];
		cur_offsets[i] = dst_offsets[i];
	}

	for(identifier_type i = 0; i < in_record_count; ++i){
		const unsigned int p = partitions[i];
		const auto record_length = in_offsets[i + 1] - in_offsets[i];
		const auto key_length = in_key_lengths[i];
		const auto total_length = record_length + 2 * sizeof(size_type);
		const auto dst_ptr = reinterpret_cast<size_type *>(
			reinterpret_cast<uint8_t *>(dst_sb.data()) + cur_offsets[p]);
		dst_ptr[0] = record_length;
		dst_ptr[1] = key_length;
		memcpy(dst_ptr + 2, in_data + in_offsets[i], record_length);
		cur_offsets[p] += total_length;
	}

	std::lock_guard<std::mutex> lock(m_mutex);
	m_partitioned_buffers.emplace_back(
		MemoryReference(dst_sb.raw_reference()));
}

void ShuffleLogicalTask::create_sort_tasks(ExecutionContext &context){
	auto &scheduler = context.scheduler();
	// create sort tasks
	for(identifier_type p = 0; p < m_partition_count; ++p){
		const auto pid = scheduler.create_physical_task(
			task_id(),
			std::unique_ptr<PhysicalTaskCommandBase>(
				new ShuffleSortCommand(
					this, p, m_partitioned_buffers)),
			LocalityOption());
		scheduler
			.add_dependency(barrier_task(), pid)
			.add_dependency(pid, terminal_task());
		scheduler.commit_task(pid);
	}
	m_partitioned_buffers.clear();
}

void ShuffleLogicalTask::sort_records(
	ExecutionContext &context,
	std::vector<LockedMemoryReference> mobjs,
	identifier_type partition)
{
	auto &memory_manager = context.memory_manager();
	const size_type fragment_count = mobjs.size();
	std::vector<ShuffleBuffer> src_sb(fragment_count);
	size_type total_record_count = 0;
	for(identifier_type i = 0; i < fragment_count; ++i){
		src_sb[i] = ShuffleBuffer(std::move(mobjs[i]), m_partition_count);
		auto data =
			static_cast<const uint8_t *>(src_sb[i].data()) +
			src_sb[i].offsets()[partition];
		const auto data_end =
			static_cast<const uint8_t *>(src_sb[i].data()) +
			src_sb[i].offsets()[partition + 1];
		while(data != data_end){
			const auto size = *reinterpret_cast<const size_type *>(data);
			data += size + 2 * sizeof(size_type);
			++total_record_count;
		}
	}

	std::vector<uint8_t> equals_to_left(total_record_count);
	std::vector<cache_type> front_cache(total_record_count);
	std::vector<cache_type> back_cache(total_record_count);
	std::vector<const uint8_t *> front_pointers(total_record_count);
	std::vector<const uint8_t *> back_pointers(total_record_count);
	for(identifier_type i = 0, k = 0; i < fragment_count; ++i){
		auto data =
			static_cast<const uint8_t *>(src_sb[i].data()) +
			src_sb[i].offsets()[partition];
		const auto data_end =
			static_cast<const uint8_t *>(src_sb[i].data()) +
			src_sb[i].offsets()[partition + 1];
		while(data != data_end){
			const auto size = *reinterpret_cast<const size_type *>(data);
			front_pointers[k++] = data;
			data += size + 2 * sizeof(size_type);
		}
	}
	msd_radix_sort(
		equals_to_left.data(),
		front_cache.data(), front_pointers.data(),
		back_cache.data(), back_pointers.data(),
		total_record_count);

	size_type group_count = 0, total_key_size = 0, total_value_size = 0;
	for(identifier_type i = 0; i < total_record_count; ++i){
		const size_type *ptr =
			reinterpret_cast<const size_type *>(front_pointers[i]);
		const auto key_length = ptr[1];
		const auto value_length = ptr[0] - ptr[1];
		total_value_size += value_length;
		if(!equals_to_left[i]){
			total_key_size += key_length;
			++group_count;
		}
	}
	auto &locality_manager = context.locality_manager();
	SerializedBuffer dst_sb = SerializedBuffer::allocate_grouped_buffer(
		memory_manager, total_record_count,
		group_count, total_key_size, total_value_size,
		locality_manager.partition_mapping(partition));
	const auto dst_keys = static_cast<uint8_t *>(dst_sb.keys_data());
	const auto dst_values = static_cast<uint8_t *>(dst_sb.values_data());
	auto dst_keys_offsets = dst_sb.keys_offsets();
	auto dst_values_offsets = dst_sb.values_offsets();
	auto dst_group_offsets = dst_sb.value_group_offsets();
	dst_keys_offsets[0] = dst_values_offsets[0] = dst_group_offsets[0] = 0;
	for(identifier_type i = 0, j = 0; i < total_record_count; ++i){
		const auto ptr =
			reinterpret_cast<const size_type *>(front_pointers[i]);
		const auto key_length = ptr[1];
		const auto value_length = ptr[0] - ptr[1];
		const auto key_ptr = reinterpret_cast<const uint8_t *>(ptr + 2);
		if(!equals_to_left[i]){
			memcpy(dst_keys + dst_keys_offsets[j], key_ptr, key_length);
			dst_keys_offsets[j + 1] = dst_keys_offsets[j] + key_length;
			dst_group_offsets[j + 1] = dst_group_offsets[j];
			++j;
		}
		const auto value_ptr = key_ptr + key_length;
		memcpy(
			dst_values + dst_values_offsets[i], value_ptr, value_length);
		dst_values_offsets[i + 1] = dst_values_offsets[i] + value_length;
		dst_group_offsets[j] += value_length;
	}
	dst_sb.record_count(total_record_count);
	commit_fragment(
		context, 0, partition, MemoryReference(dst_sb.raw_reference()));
}


void ShuffleLogicalTask::receive_fragment(
	ExecutionContext &context,
	identifier_type port,
	identifier_type partition,
	MemoryReference mobj)
{
	(void)(port);
	assert(port == 0);
	(void)(partition);
	assert(partition == 0);
	auto &scheduler = context.scheduler();
	auto &locality_manager = context.locality_manager();
	const auto mobj_loc = mobj.locality();
	const auto pid = scheduler.create_physical_task(
		task_id(),
		std::unique_ptr<PhysicalTaskCommandBase>(
			new ShufflePartitionCommand(this, std::move(mobj))),
		LocalityOption(
			locality_manager.random_worker_from_node(mobj_loc)));
	scheduler
		.add_dependency(entry_task(), pid)
		.add_dependency(pid, barrier_task());
	scheduler.commit_task(pid);
}

}

