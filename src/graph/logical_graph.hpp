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
#ifndef M3BP_GRAPH_LOGICAL_GRAPH_HPP
#define M3BP_GRAPH_LOGICAL_GRAPH_HPP

#include <vector>
#include <limits>
#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/range.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include "m3bp/types.hpp"
#include "common/array_ref.hpp"
#include "tasks/logical_task_base.hpp"
#include "tasks/logical_task_identifier.hpp"

namespace m3bp {

class ExecutionContext;

class LogicalGraph {

public:
	static const identifier_type INVALID_ID =
		std::numeric_limits<identifier_type>::max();


	using LogicalTaskPtr = std::shared_ptr<LogicalTaskBase>;
	using LogicalTaskList = std::vector<LogicalTaskPtr>;

	using LogicalTaskListIterator =
		boost::indirect_iterator<LogicalTaskList::const_iterator>;
	using LogicalTaskListRange =
		boost::iterator_range<LogicalTaskListIterator>;

	using ConstLogicalTaskListIterator =
		boost::indirect_iterator<
			LogicalTaskList::const_iterator, const LogicalTaskBase>;
	using ConstLogicalTaskListRange =
		boost::iterator_range<ConstLogicalTaskListIterator>;


	class Port {
	private:
		LogicalTaskIdentifier m_task_id;
		identifier_type m_port_id;
	public:
		Port() noexcept
			: m_task_id()
			, m_port_id(INVALID_ID)
		{ }
		Port(LogicalTaskIdentifier task_id, identifier_type port_id) noexcept
			: m_task_id(task_id)
			, m_port_id(port_id)
		{ }
		LogicalTaskIdentifier task_id() const noexcept { return m_task_id; }
		identifier_type port_id() const noexcept { return m_port_id; }
	};

	enum class PhysicalSuccessor {
		ENTRY, BARRIER, TERMINAL
	};

	class Edge {
	private:
		Port m_producer_port;
		Port m_consumer_port;
		PhysicalSuccessor m_physical_successor;
	public:
		Edge() noexcept
			: m_producer_port()
			, m_consumer_port()
			, m_physical_successor(PhysicalSuccessor::ENTRY)
		{ }
		Edge(
			Port producer, Port consumer,
			PhysicalSuccessor physical_successor) noexcept
			: m_producer_port(producer)
			, m_consumer_port(consumer)
			, m_physical_successor(physical_successor)
		{ }
		const Port &producer() const noexcept { return m_producer_port; }
		const Port &consumer() const noexcept { return m_consumer_port; }
		PhysicalSuccessor physical_successor() const noexcept {
			return m_physical_successor;
		}
	};

	using EdgeList = std::vector<Edge>;
	using ConstEdgeListRange =
		boost::iterator_range<EdgeList::const_iterator>;

private:
	std::vector<LogicalTaskPtr> m_logical_tasks;
	std::vector<Edge> m_edges;

public:
	LogicalGraph();
	LogicalGraph(const LogicalGraph &) = delete;
	LogicalGraph(LogicalGraph &&) = default;

	LogicalGraph &operator=(const LogicalGraph &) = delete;
	LogicalGraph &operator=(LogicalGraph &&) = default;

	LogicalTaskIdentifier add_logical_task(LogicalTaskPtr task);
	LogicalGraph &add_edge(
		Port producer, Port consumer, PhysicalSuccessor physical_successor);

	void create_physical_tasks(ExecutionContext &context);

	ConstLogicalTaskListRange logical_tasks() const;
	LogicalTaskListRange logical_tasks();

	ConstEdgeListRange logical_edges() const;

};

}

#endif

