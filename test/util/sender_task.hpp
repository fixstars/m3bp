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
#ifndef M3BP_TEST_UTIL_SENDER_TASK_HPP
#define M3BP_TEST_UTIL_SENDER_TASK_HPP

#include <vector>
#include <memory>
#include "scheduler/locality_option.hpp"
#include "tasks/logical_task_base.hpp"
#include "tasks/physical_task_command_base.hpp"
#include "context/execution_context.hpp"
#include "util/serialize_util.hpp"

namespace util {

template <typename T>
class SenderPhysicalTaskCommand : public m3bp::PhysicalTaskCommandBase {
private:
	m3bp::LogicalTaskBase *m_logical_task;
	std::vector<T> m_original_data;
public:
	template <typename Iterator>
	SenderPhysicalTaskCommand(
		m3bp::LogicalTaskBase *logical_task, Iterator first, Iterator last)
		: m_logical_task(logical_task)
		, m_original_data(first, last)
	{ }
	virtual void run(
		m3bp::ExecutionContext &context,
		const m3bp::Locality & /* locality */) override
	{
		auto &memory_manager = context.memory_manager();
		auto mobj = create_serialized_buffer(memory_manager, m_original_data);
		m_logical_task->commit_fragment(
			context, 0, 0, m3bp::MemoryReference(mobj));
	}
};

class SenderBarrierTaskCommand : public m3bp::PhysicalTaskCommandBase {
public:
	virtual void run(
		m3bp::ExecutionContext & /* context */,
		const m3bp::Locality & /* locality */) override
	{ }
};

template <typename T>
class SenderTask : public m3bp::LogicalTaskBase {
private:
	std::vector<std::vector<T>> m_original_data;
public:
	template <typename Iterator>
	explicit SenderTask(Iterator first, Iterator last)
		: m_original_data()
	{
		for(Iterator it = first; it != last; ++it){
			m_original_data.emplace_back(std::begin(*it), std::end(*it));
		}
	}
	virtual void create_physical_tasks(
		m3bp::ExecutionContext &context) override
	{
		auto &scheduler = context.scheduler();
		const auto logical_id = task_id();
		// create barriers
		const auto entry_id = scheduler.create_physical_task(
			logical_id,
			std::unique_ptr<m3bp::PhysicalTaskCommandBase>(
				new SenderBarrierTaskCommand()),
			m3bp::LocalityOption());
		const auto terminal_id = scheduler.create_physical_task(
			logical_id,
			std::unique_ptr<m3bp::PhysicalTaskCommandBase>(
				new SenderBarrierTaskCommand()),
			m3bp::LocalityOption());
		scheduler.add_dependency(entry_id, terminal_id);
		// register barriers (barrier == terminal)
		entry_task   (entry_id);
		barrier_task (terminal_id);
		terminal_task(terminal_id);
		// create sender tasks
		for(const auto &data : m_original_data){
			const auto pid = scheduler.create_physical_task(
				logical_id,
				std::unique_ptr<m3bp::PhysicalTaskCommandBase>(
					new SenderPhysicalTaskCommand<T>(
						this, data.begin(), data.end())),
				m3bp::LocalityOption());
			scheduler
				.add_dependency(entry_id, pid)
				.add_dependency(pid, terminal_id);
			scheduler.commit_task(pid);
		}
	}
	virtual void commit_physical_tasks(
		m3bp::ExecutionContext &context) override
	{
		auto &scheduler = context.scheduler();
		// barrier_task == terminal_task
		scheduler.commit_task(entry_task());
		scheduler.commit_task(terminal_task());
	}
};

}

#endif

