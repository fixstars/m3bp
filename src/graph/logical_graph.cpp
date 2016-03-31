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
#include "graph/logical_graph.hpp"
#include "context/execution_context.hpp"
#include "logging/general_logger.hpp"

#define M3BP_LOGICAL_GRAPH_TRACE \
	M3BP_GENERAL_LOG(TRACE) << "[LogicalGraph] [" << __func__ << "] "

namespace m3bp {

LogicalGraph::LogicalGraph()
	: m_logical_tasks()
	, m_edges()
{ }


LogicalTaskIdentifier LogicalGraph::add_logical_task(LogicalTaskPtr task){
	assert(task);
	const LogicalTaskIdentifier task_id(m_logical_tasks.size());
	m_logical_tasks.emplace_back(std::move(task));
	m_logical_tasks.back()->task_id(task_id);
	M3BP_LOGICAL_GRAPH_TRACE
		<< task_id.identifier() << " " << m_logical_tasks.back()->task_name();
	return task_id;
}

LogicalGraph &LogicalGraph::add_edge(
	Port producer, Port consumer, PhysicalSuccessor physical_successor)
{
	assert(producer.task_id().identifier() < m_logical_tasks.size());
	assert(consumer.task_id().identifier() < m_logical_tasks.size());
	m_edges.emplace_back(producer, consumer, physical_successor);
	m_logical_tasks[producer.task_id().identifier()]->add_successor(
		m_logical_tasks[consumer.task_id().identifier()],
		consumer.port_id(), producer.port_id());
	M3BP_LOGICAL_GRAPH_TRACE
		<< producer.task_id().identifier() << " " << producer.port_id() << " "
		<< consumer.task_id().identifier() << " " << consumer.port_id();
	return *this;
}

void LogicalGraph::create_physical_tasks(ExecutionContext &context){
	auto &scheduler = context.scheduler();
	for(auto &logical_task : m_logical_tasks){
		logical_task->create_physical_tasks(context);
	}
	for(const auto &e : m_edges){
		auto &producer = m_logical_tasks[e.producer().task_id().identifier()];
		auto &consumer = m_logical_tasks[e.consumer().task_id().identifier()];
		switch(e.physical_successor()){
			case PhysicalSuccessor::ENTRY:
				scheduler.add_dependency(
					producer->terminal_task(),
					consumer->entry_task());
				break;
			case PhysicalSuccessor::BARRIER:
				scheduler.add_dependency(
					producer->terminal_task(),
					consumer->barrier_task());
				break;
			case PhysicalSuccessor::TERMINAL:
				scheduler.add_dependency(
					producer->terminal_task(),
					consumer->terminal_task());
				break;
		}
	}
	for(auto &logical_task : m_logical_tasks){
		logical_task->commit_physical_tasks(context);
	}
}


LogicalGraph::ConstLogicalTaskListRange LogicalGraph::logical_tasks() const {
	return ConstLogicalTaskListRange(m_logical_tasks);
}

LogicalGraph::LogicalTaskListRange LogicalGraph::logical_tasks(){
	return LogicalTaskListRange(m_logical_tasks);
}

LogicalGraph::ConstEdgeListRange LogicalGraph::logical_edges() const {
	return ConstEdgeListRange(m_edges);
}

}

