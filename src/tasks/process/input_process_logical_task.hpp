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
#ifndef M3BP_TASKS_PROCESS_INPUT_PROCESS_LOGICAL_TASK_HPP
#define M3BP_TASKS_PROCESS_INPUT_PROCESS_LOGICAL_TASK_HPP

#include "tasks/process/process_task_base.hpp"

namespace m3bp {

class InputProcessLogicalTask : public ProcessLogicalTaskBase {

private:
	class InputProcessRunCommand;

public:
	InputProcessLogicalTask();
	InputProcessLogicalTask(
		internal::ProcessorWrapper pw,
		size_type worker_count);

protected:
	virtual void after_global_initialize(ExecutionContext &context) override;

private:
	void run(
		ExecutionContext &context,
		const Locality &locality,
		identifier_type physical_task_id,
		std::vector<LockedMemoryReference> inputs);

};

}

#endif

