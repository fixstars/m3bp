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
#ifndef M3BP_TASKS_GATHER_GATHER_LOGICAL_TASK_HPP
#define M3BP_TASKS_GATHER_GATHER_LOGICAL_TASK_HPP

#include <vector>
#include "tasks/logical_task_base.hpp"
#include "common/array_ref.hpp"
#include "memory/memory_reference.hpp"
#include "memory/memory_manager.hpp"

namespace m3bp {

class MemoryManager;

class GatherLogicalTask : public LogicalTaskBase {

private:
	std::mutex m_mutex;
	std::vector<MemoryReference> m_inputs;

public:
	GatherLogicalTask();

	virtual void create_physical_tasks(ExecutionContext &context) override;
	virtual void commit_physical_tasks(ExecutionContext &context) override;


	ArrayRef<MemoryReference> inputs();

	void run_gather(
		ExecutionContext &context,
		std::vector<LockedMemoryReference> locked_inputs);


protected:
	virtual void receive_fragment(
		ExecutionContext & /* context */,
		identifier_type port,
		identifier_type partition,
		MemoryReference mobj) override;

};

}

#endif

