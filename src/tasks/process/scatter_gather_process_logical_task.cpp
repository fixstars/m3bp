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
#include "m3bp/processor_base.hpp"
#include "common/make_unique.hpp"
#include "tasks/process/scatter_gather_process_logical_task.hpp"
#include "tasks/physical_task_command_base.hpp"
#include "context/execution_context.hpp"
#include "scheduler/locality_option.hpp"

namespace m3bp {

class ScatterGatherProcessLogicalTask::ScatterGatherProcessRunCommand
	: public ProcessCommandBase
{
private:
	ScatterGatherProcessLogicalTask *m_process_task;
	std::vector<MemoryReference> m_unlocked_inputs;
	std::vector<LockedMemoryReference> m_locked_inputs;
	identifier_type m_partition;

public:
	ScatterGatherProcessRunCommand(
		ScatterGatherProcessLogicalTask *process_task,
		std::vector<MemoryReference> inputs,
		identifier_type partition)
		: m_process_task(process_task)
		, m_unlocked_inputs(std::move(inputs))
		, m_locked_inputs(m_unlocked_inputs.size())
		, m_partition(partition)
	{ }

	virtual void prepare(
		ExecutionContext & /* context */,
		const Locality & /* locality */) override
	{
		assert(m_process_task);
		const auto port_count = m_unlocked_inputs.size();
		std::vector<LockedMemoryReference> locked(port_count);
		for(identifier_type i = 0; i < port_count; ++i){
			if(m_unlocked_inputs[i]){
				locked[i] = m_unlocked_inputs[i].lock();
			}
		}
		m_locked_inputs = std::move(locked);
		m_unlocked_inputs.clear();
	}

	virtual void run(
		ExecutionContext &context,
		const Locality &locality,
		std::vector<LockedMemoryReference> mobjs) override
	{
		assert(m_process_task);
		const auto port_count = m_locked_inputs.size();
		assert(port_count == mobjs.size());
		for(identifier_type i = 0; i < port_count; ++i){
			if(mobjs[i]){
				assert(!m_locked_inputs[i]);
			}else{
				mobjs[i] = std::move(m_locked_inputs[i]);
			}
		}
		m_process_task->run(
			context, locality, std::move(mobjs), m_partition);
	}
};


ScatterGatherProcessLogicalTask::ScatterGatherProcessLogicalTask()
	: ProcessLogicalTaskBase()
	, m_partitioned_pool()
	, m_waiting_counters()
{ }

ScatterGatherProcessLogicalTask::ScatterGatherProcessLogicalTask(
	internal::ProcessorWrapper pw,
	size_type worker_count,
	size_type partition_count)
	: ProcessLogicalTaskBase(std::move(pw), worker_count)
	, m_partitioned_pool(
		partition_count,
		std::vector<MemoryReference>(processor().input_ports().size()))
	, m_waiting_counters(partition_count)
{
	size_type scatter_gather_count = 0;
	for(const auto &port : processor().input_ports()){
		if(port.movement() == Movement::SCATTER_GATHER){
			++scatter_gather_count;
		}
	}
	for(identifier_type i = 0; i < partition_count; ++i){
		m_waiting_counters[i].store(scatter_gather_count);
	}
}


void ScatterGatherProcessLogicalTask::receive_non_broadcast_fragment(
	ExecutionContext &context,
	identifier_type port,
	identifier_type partition,
	MemoryReference mobj)
{
	assert(partition < m_partitioned_pool.size());
	assert(port < m_partitioned_pool[partition].size());
	assert(
		processor().input_ports()[port].movement() ==
			Movement::SCATTER_GATHER);
	auto &scheduler = context.scheduler();
	m_partitioned_pool[partition][port] = std::move(mobj);
	if(--m_waiting_counters[partition] == 0){
		auto &locality_manager = context.locality_manager();
		const auto pid = scheduler.create_physical_task(
			task_id(),
			make_unique<ProcessCommandWrapper>(
				this, make_unique<ScatterGatherProcessRunCommand>(
					this, std::move(m_partitioned_pool[partition]),
					partition)),
			LocalityOption(
				locality_manager.random_worker_from_node(
					locality_manager.partition_mapping(partition))));
		scheduler
			.add_dependency(entry_task(), pid)
			.add_dependency(pid, barrier_task());
		commit_process_command(context, pid);
	}
}

void ScatterGatherProcessLogicalTask::run(
	ExecutionContext &context,
	const Locality &locality,
	std::vector<LockedMemoryReference> inputs,
	identifier_type partition)
{
	thread_local_initialize(context, locality, inputs);
	auto task = create_task_object(
		context, std::move(inputs), partition, false, locality);
	processor().run(task);
}

}

