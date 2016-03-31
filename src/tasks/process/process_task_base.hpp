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
#ifndef M3BP_TASKS_PROCESS_PROCESS_TASK_BASE_HPP
#define M3BP_TASKS_PROCESS_PROCESS_TASK_BASE_HPP

#include <memory>
#include <atomic>
#include <mutex>
#include <queue>
#include "m3bp/task.hpp"
#include "m3bp/internal/processor_wrapper.hpp"
#include "tasks/logical_task_base.hpp"
#include "tasks/physical_task_command_base.hpp"

namespace m3bp {

class ProcessLogicalTaskBase : public LogicalTaskBase {

protected:
	class ProcessCommandBase {
	public:
		virtual ~ProcessCommandBase(){ }

		virtual void prepare(
			ExecutionContext & /* context */,
			const Locality &   /* locality */)
		{ }

		virtual void run(
			ExecutionContext & /* context */,
			const Locality &   /* locality */,
			std::vector<LockedMemoryReference> /* mobjs */)
		{ }
	};

	class ProcessCommandWrapper
		: public PhysicalTaskCommandBase
	{
	private:
		ProcessLogicalTaskBase *m_logical_task;
		std::unique_ptr<ProcessCommandBase> m_command;
		std::vector<LockedMemoryReference> m_broadcast_inputs;

	public:
		ProcessCommandWrapper(
			ProcessLogicalTaskBase *logical_task,
			std::unique_ptr<ProcessCommandBase> command);

		virtual void prepare(
			ExecutionContext &context,
			const Locality &locality) override;

		virtual void run(
			ExecutionContext &context,
			const Locality &locality) override;
	};

private:
	class GlobalInitializeCommand;
	class GlobalFinalizeCommand;
	class ThreadLocalFinalizeCommand;
	class ProcessBarrierCommand;

	internal::ProcessorWrapper m_processor;

	bool m_global_initialized;
	std::vector<int> m_thread_local_initialized;

	std::vector<MemoryReference> m_broadcast_inputs;

	std::mutex m_local_queue_mutex;
	size_type m_remaining_concurrency;
	std::queue<PhysicalTaskIdentifier> m_task_queue;

public:
	ProcessLogicalTaskBase();
	ProcessLogicalTaskBase(
		internal::ProcessorWrapper pw,
		size_type worker_count);

	virtual void create_physical_tasks(ExecutionContext &context) override;
	virtual void commit_physical_tasks(ExecutionContext &context) override;


	virtual void thread_local_cancel(
		ExecutionContext &context,
		const Locality &locality) override;

	virtual void global_cancel(ExecutionContext &context) override;


protected:
	virtual void receive_fragment(
		ExecutionContext &context,
		identifier_type port,
		identifier_type partition,
		MemoryReference mobj) override;

	virtual void receive_non_broadcast_fragment(
		ExecutionContext & /* context */,
		identifier_type /* port */,
		identifier_type /* partition */,
		MemoryReference /* mobj */)
	{ }


	const ProcessorBase &processor() const;
	ProcessorBase &processor();


	void commit_process_command(
		ExecutionContext &context,
		PhysicalTaskIdentifier task);


	Task create_task_object(
		ExecutionContext &context,
		std::vector<LockedMemoryReference> input_buffers,
		identifier_type physical_task_id,
		bool is_cancelled,
		const Locality &locality);


	virtual void after_global_initialize(ExecutionContext & /* context */){ }


	void global_initialize(
		ExecutionContext &context,
		const Locality &locality,
		std::vector<LockedMemoryReference> mobjs);

	void global_finalize(
		ExecutionContext &context,
		const Locality &locality,
		std::vector<LockedMemoryReference> mobjs);

	void thread_local_initialize(
		ExecutionContext &context,
		const Locality &locality,
		std::vector<LockedMemoryReference> mobjs);

	void thread_local_finalize(
		ExecutionContext &context,
		const Locality &locality,
		std::vector<LockedMemoryReference> mobjs);


private:
	void create_thread_local_finalizers(ExecutionContext &context);

	void notify_completion(ExecutionContext &context);

	std::vector<MemoryReference> broadcast_inputs();

};

}

#endif

