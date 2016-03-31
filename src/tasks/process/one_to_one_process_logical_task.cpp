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
#include "tasks/process/one_to_one_process_logical_task.hpp"
#include "tasks/physical_task_command_base.hpp"
#include "context/execution_context.hpp"
#include "scheduler/locality_option.hpp"

namespace m3bp {

class OneToOneProcessLogicalTask::OneToOneProcessRunCommand
	: public ProcessCommandBase
{
private:
	OneToOneProcessLogicalTask *m_process_task;
	MemoryReference m_one_to_one_input;
	identifier_type m_one_to_one_port;
	LockedMemoryReference m_locked_input;

public:
	OneToOneProcessRunCommand(
		OneToOneProcessLogicalTask *process_task,
		MemoryReference one_to_one_input,
		identifier_type one_to_one_port)
		: m_process_task(process_task)
		, m_one_to_one_input(std::move(one_to_one_input))
		, m_one_to_one_port(one_to_one_port)
		, m_locked_input()
	{ }

	virtual void prepare(
		ExecutionContext & /* context */,
		const Locality & /* locality */) override
	{
		assert(m_process_task);
		m_locked_input = m_one_to_one_input.lock();
		m_one_to_one_input = MemoryReference();
	}

	virtual void run(
		ExecutionContext &context,
		const Locality &locality,
		std::vector<LockedMemoryReference> mobjs) override
	{
		assert(m_process_task);
		assert(m_one_to_one_port < mobjs.size());
		mobjs[m_one_to_one_port] = std::move(m_locked_input);
		m_process_task->run(context, locality, std::move(mobjs));
	}
};


OneToOneProcessLogicalTask::OneToOneProcessLogicalTask()
	: ProcessLogicalTaskBase()
	, m_next_physical_id()
{ }

OneToOneProcessLogicalTask::OneToOneProcessLogicalTask(
	internal::ProcessorWrapper pw,
	size_type worker_count)
	: ProcessLogicalTaskBase(std::move(pw), worker_count)
	, m_next_physical_id()
{ }


void OneToOneProcessLogicalTask::receive_non_broadcast_fragment(
	ExecutionContext &context,
	identifier_type port,
	identifier_type partition,
	MemoryReference mobj)
{
	(void)(partition);
	assert(partition == 0);
	assert(processor().input_ports()[port].movement() == Movement::ONE_TO_ONE);
	auto &scheduler = context.scheduler();
	auto &locality_manager = context.locality_manager();
	const auto mobj_loc = mobj.locality();
	const auto pid = scheduler.create_physical_task(
		task_id(),
		make_unique<ProcessCommandWrapper>(
			this, make_unique<OneToOneProcessRunCommand>(
				this, std::move(mobj), port)),
		LocalityOption(
			locality_manager.random_worker_from_node(mobj_loc)));
	scheduler
		.add_dependency(entry_task(), pid)
		.add_dependency(pid, barrier_task());
	commit_process_command(context, pid);
}

void OneToOneProcessLogicalTask::run(
	ExecutionContext &context,
	const Locality &locality,
	std::vector<LockedMemoryReference> inputs)
{
	thread_local_initialize(context, locality, inputs);
	auto task = create_task_object(
		context, std::move(inputs), m_next_physical_id++, false, locality);
	processor().run(task);
}

}

