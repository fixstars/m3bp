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
#include "tasks/process/input_process_logical_task.hpp"
#include "tasks/physical_task_command_base.hpp"
#include "context/execution_context.hpp"
#include "scheduler/locality_option.hpp"

namespace m3bp {

class InputProcessLogicalTask::InputProcessRunCommand
	: public ProcessCommandBase 
{
private:
	InputProcessLogicalTask *m_process_task;
	identifier_type m_physical_task_id;

public:
	InputProcessRunCommand(
		InputProcessLogicalTask *process_task,
		size_type physical_task_id)
		: m_process_task(process_task)
		, m_physical_task_id(physical_task_id)
	{ }

	virtual void run(
		ExecutionContext &context,
		const Locality &locality,
		std::vector<LockedMemoryReference> mobjs) override
	{
		assert(m_process_task);
		m_process_task->run(
			context, locality, m_physical_task_id, std::move(mobjs));
	}
};


InputProcessLogicalTask::InputProcessLogicalTask()
	: ProcessLogicalTaskBase()
{ }

InputProcessLogicalTask::InputProcessLogicalTask(
	internal::ProcessorWrapper pw,
	size_type worker_count)
	: ProcessLogicalTaskBase(std::move(pw), worker_count)
{ }


void InputProcessLogicalTask::after_global_initialize(
	ExecutionContext &context)
{
	auto &scheduler = context.scheduler();
	const auto task_count = processor().task_count();
	for(identifier_type i = 0; i < task_count; ++i){
		const auto pid = scheduler.create_physical_task(
			task_id(),
			make_unique<ProcessCommandWrapper>(
				this, make_unique<InputProcessRunCommand>(this, i)),
			LocalityOption());
		scheduler
			.add_dependency(entry_task(), pid)
			.add_dependency(pid, barrier_task());
		commit_process_command(context, pid);
	}
}

void InputProcessLogicalTask::run(
	ExecutionContext &context,
	const Locality &locality,
	identifier_type physical_task_id,
	std::vector<LockedMemoryReference> inputs)
{
	thread_local_initialize(context, locality, inputs);
	auto task = create_task_object(
		context, std::move(inputs), physical_task_id, false, locality);
	processor().run(task);
}

}

