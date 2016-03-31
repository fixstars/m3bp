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
#include "tasks/physical_task.hpp"
#include "tasks/physical_task_command_base.hpp"
#include "context/execution_context.hpp"

namespace m3bp {

PhysicalTask::PhysicalTask()
	: m_logical_task_id()
	, m_physical_task_id()
	, m_command()
{ }

PhysicalTask::PhysicalTask(
	LogicalTaskIdentifier logical_id,
	PhysicalTaskIdentifier physical_id,
	CommandPtr command)
	: m_logical_task_id(std::move(logical_id))
	, m_physical_task_id(std::move(physical_id))
	, m_command(std::move(command))
{ }

void PhysicalTask::prepare(
	ExecutionContext &context,
	const Locality &locality)
{
	assert(m_command);
	m_command->prepare(context, locality);
}

void PhysicalTask::run(
	ExecutionContext &context,
	const Locality &locality)
{
	assert(m_command);
	m_command->run(context, locality);
}

}

