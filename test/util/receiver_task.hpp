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
#ifndef M3BP_TEST_UTIL_RECEIVER_TASK_HPP
#define M3BP_TEST_UTIL_RECEIVER_TASK_HPP

#include <mutex>
#include "tasks/logical_task_base.hpp"
#include "tasks/physical_task_command_base.hpp"
#include "context/execution_context.hpp"
#include "scheduler/locality_option.hpp"
#include "util/serialize_util.hpp"

namespace util {

class ReceiverBarrierTaskCommand : public m3bp::PhysicalTaskCommandBase {
public:
	virtual void run(
		m3bp::ExecutionContext & /* context */,
		const m3bp::Locality & /* locality */) override
	{ }
};

template <typename T>
class ReceiverTask : public m3bp::LogicalTaskBase {
private:
	std::mutex m_mutex;
	std::vector<std::vector<T>> m_received_data;
public:
	explicit ReceiverTask(m3bp::size_type partition_count)
		: m3bp::LogicalTaskBase()
		, m_mutex()
		, m_received_data(partition_count)
	{ }
	const std::vector<T> &received_data(
		m3bp::identifier_type partition) const noexcept
	{
		return m_received_data[partition];
	}
	virtual void create_physical_tasks(
		m3bp::ExecutionContext &context) override
	{
		auto &scheduler = context.scheduler();
		const auto pid = scheduler.create_physical_task(
			task_id(),
			std::unique_ptr<m3bp::PhysicalTaskCommandBase>(
				new ReceiverBarrierTaskCommand()),
			m3bp::LocalityOption());
		entry_task(pid);
		barrier_task(pid);
		terminal_task(pid);
	}
	virtual void commit_physical_tasks(
		m3bp::ExecutionContext &context) override
	{
		auto &scheduler = context.scheduler();
		scheduler.commit_task(entry_task());
	}
protected:
	virtual void receive_fragment(
		m3bp::ExecutionContext & /* context */,
		m3bp::identifier_type port,
		m3bp::identifier_type partition,
		m3bp::MemoryReference mobj) override
	{
		EXPECT_EQ(0u, port);
		std::vector<T> received;
		received = deserialize_value_only_buffer<T>(mobj.lock());
		std::lock_guard<std::mutex> lock(m_mutex);
		for(auto &x : received){
			m_received_data[partition].emplace_back(std::move(x));
		}
	}
};

template <typename KeyType, typename ValueType>
class GroupedReceiverTask : public m3bp::LogicalTaskBase {
public:
	using GroupType = std::pair<KeyType, std::vector<ValueType>>;
private:
	std::mutex m_mutex;
	std::vector<std::vector<GroupType>> m_received_data;
public:
	explicit GroupedReceiverTask(m3bp::size_type partition_count)
		: m_mutex()
		, m_received_data(partition_count)
	{ }
	const std::vector<GroupType> &received_data(
		m3bp::identifier_type partition) const noexcept
	{
		return m_received_data[partition];
	}
	virtual void create_physical_tasks(
		m3bp::ExecutionContext &context) override
	{
		auto &scheduler = context.scheduler();
		const auto pid = scheduler.create_physical_task(
			task_id(),
			std::unique_ptr<m3bp::PhysicalTaskCommandBase>(
				new ReceiverBarrierTaskCommand()),
			m3bp::LocalityOption());
		entry_task(pid);
		barrier_task(pid);
		terminal_task(pid);
	}
	virtual void commit_physical_tasks(
		m3bp::ExecutionContext &context) override
	{
		auto &scheduler = context.scheduler();
		scheduler.commit_task(entry_task());
	}
protected:
	virtual void receive_fragment(
		m3bp::ExecutionContext & /* context */,
		m3bp::identifier_type port,
		m3bp::identifier_type partition,
		m3bp::MemoryReference mobj) override
	{
		EXPECT_EQ(0u, port);
		std::vector<std::pair<KeyType, std::vector<ValueType>>> received;
		received = deserialize_grouped_buffer<KeyType, ValueType>(mobj.lock());
		std::lock_guard<std::mutex> lock(m_mutex);
		for(auto &x : received){
			m_received_data[partition].emplace_back(std::move(x));
		}
	}
};

}

#endif

