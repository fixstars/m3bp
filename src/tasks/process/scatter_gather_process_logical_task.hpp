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
#ifndef M3BP_TASKS_PROCESS_SCATTER_GATHER_PROCESS_LOGICAL_TASK_HPP
#define M3BP_TASKS_PROCESS_SCATTER_GATHER_PROCESS_LOGICAL_TASK_HPP

#include <atomic>
#include "common/noncopyable_vector.hpp"
#include "tasks/process/process_task_base.hpp"

namespace m3bp {

class ScatterGatherProcessLogicalTask : public ProcessLogicalTaskBase {

private:
	class ScatterGatherProcessRunCommand;

	std::vector<std::vector<MemoryReference>> m_partitioned_pool;
	NoncopyableVector<std::atomic<size_type>> m_waiting_counters;

public:
	ScatterGatherProcessLogicalTask();
	ScatterGatherProcessLogicalTask(
		internal::ProcessorWrapper pw,
		size_type worker_count,
		size_type partition_count);

protected:
	virtual void receive_non_broadcast_fragment(
		ExecutionContext &context,
		identifier_type port,
		identifier_type partition,
		MemoryReference mobj) override;

private:
	void run(
		ExecutionContext &context,
		const Locality &locality,
		std::vector<LockedMemoryReference> inputs,
		identifier_type partition);

};

}

#endif

