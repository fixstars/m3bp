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
#ifndef M3BP_TASKS_LOGICAL_TASK_BASE_HPP
#define M3BP_TASKS_LOGICAL_TASK_BASE_HPP

#include <memory>
#include <vector>
#include "tasks/logical_task_identifier.hpp"
#include "tasks/physical_task_identifier.hpp"
#include "memory/memory_reference.hpp"

namespace m3bp {

class ExecutionContext;
class Locality;

class LogicalTaskBase {

public:
	using LogicalTaskPtr = std::shared_ptr<LogicalTaskBase>;

private:
	using SuccessorPort = std::pair<LogicalTaskPtr, identifier_type>;

	LogicalTaskIdentifier m_task_id;
	std::string m_task_name;

	std::vector<std::vector<SuccessorPort>> m_successors;

	PhysicalTaskIdentifier m_entry_task;
	PhysicalTaskIdentifier m_barrier_task;
	PhysicalTaskIdentifier m_terminal_task;

protected:
	LogicalTaskBase();

	LogicalTaskBase(const LogicalTaskBase &) = delete;
	LogicalTaskBase &operator=(const LogicalTaskBase &) = delete;

public:
	virtual ~LogicalTaskBase() = default;

	LogicalTaskIdentifier task_id() const noexcept {
		return m_task_id;
	}
	LogicalTaskBase &task_id(LogicalTaskIdentifier id) noexcept {
		m_task_id = id;
		return *this;
	}

	const std::string &task_name() const noexcept {
		return m_task_name;
	}
	LogicalTaskBase &task_name(std::string name){
		m_task_name = std::move(name);
		return *this;
	}


	virtual void create_physical_tasks(ExecutionContext &context) = 0;
	virtual void commit_physical_tasks(ExecutionContext &context) = 0;


	PhysicalTaskIdentifier entry_task() const noexcept {
		return m_entry_task;
	}
	PhysicalTaskIdentifier barrier_task() const noexcept {
		return m_barrier_task;
	}
	PhysicalTaskIdentifier terminal_task() const noexcept {
		return m_terminal_task;
	}


	LogicalTaskBase &add_successor(
		LogicalTaskPtr dst_task,
		identifier_type dst_port,
		identifier_type src_port);

	void commit_fragment(
		ExecutionContext &context,
		identifier_type port,
		identifier_type partition,
		MemoryReference mobj);


	virtual void thread_local_cancel(
		ExecutionContext & /* context */,
		const Locality &   /* locality */)
	{ }

	virtual void global_cancel(ExecutionContext & /* context */){ }


protected:
	LogicalTaskBase &entry_task(PhysicalTaskIdentifier id) noexcept {
		m_entry_task = id;
		return *this;
	}
	LogicalTaskBase &barrier_task(PhysicalTaskIdentifier id) noexcept {
		m_barrier_task = id;
		return *this;
	}
	LogicalTaskBase &terminal_task(PhysicalTaskIdentifier id) noexcept {
		m_terminal_task = id;
		return *this;
	}

	virtual void receive_fragment(
		ExecutionContext & /* context */,
		identifier_type /* port */,
		identifier_type /* partition */,
		MemoryReference /* mobj */)
	{ }

};

}

#endif

