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
#ifndef M3BP_TASKS_PHYSICAL_TASK_HPP
#define M3BP_TASKS_PHYSICAL_TASK_HPP

#include <memory>
#include "tasks/logical_task_identifier.hpp"
#include "tasks/physical_task_identifier.hpp"

namespace m3bp {

class ExecutionContext;
class PhysicalTaskCommandBase;
class Locality;

class PhysicalTask {

public:
	using CommandPtr = std::unique_ptr<PhysicalTaskCommandBase>;

private:
	LogicalTaskIdentifier m_logical_task_id;
	PhysicalTaskIdentifier m_physical_task_id;
	CommandPtr m_command;

public:
	PhysicalTask();
	PhysicalTask(
		LogicalTaskIdentifier logical_id,
		PhysicalTaskIdentifier physical_id,
		CommandPtr command);


	LogicalTaskIdentifier logical_task_id() const noexcept {
		return m_logical_task_id;
	}
	PhysicalTaskIdentifier physical_task_id() const noexcept {
		return m_physical_task_id;
	}

	void prepare(
		ExecutionContext &context,
		const Locality &locality);

	void run(
		ExecutionContext &context,
		const Locality &locality);

};

}

#endif

