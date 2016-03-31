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
#ifndef M3BP_TASKS_SHUFFLE_SHUFFLE_LOGICAL_TASK_HPP
#define M3BP_TASKS_SHUFFLE_SHUFFLE_LOGICAL_TASK_HPP

#include <vector>
#include <mutex>
#include <memory>
#include "tasks/logical_task_base.hpp"
#include "memory/memory_reference.hpp"

namespace m3bp {

class Scheduler;
class MemoryManager;
class ShuffleBuffer;

class ShuffleLogicalTask : public LogicalTaskBase {

private:
	class InProgressBuffer;

	std::mutex m_mutex;
	size_type m_partition_count;
	std::vector<MemoryReference> m_partitioned_buffers;

public:
	explicit ShuffleLogicalTask(size_type partition_count);

	virtual void create_physical_tasks(ExecutionContext &context) override;
	virtual void commit_physical_tasks(ExecutionContext &context) override;


	void partition_fragment(
		ExecutionContext &context,
		const Locality &locality,
		LockedMemoryReference mobj);

	void create_sort_tasks(ExecutionContext &context);

	void sort_records(
		ExecutionContext &context,
		std::vector<LockedMemoryReference> mobjs,
		identifier_type partition);

protected:
	virtual void receive_fragment(
		ExecutionContext &context,
		identifier_type port,
		identifier_type partition,
		MemoryReference mobj) override;

};

}

#endif

