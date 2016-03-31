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
#include "common/make_unique.hpp"
#include "context/execution_context.hpp"
#include "m3bp/configuration.hpp"
#include "graph/logical_graph.hpp"
#include "graph/logical_graph_builder.hpp"
#include "scheduler/scheduler.hpp"
#include "scheduler/locality_manager.hpp"
#include "memory/memory_manager.hpp"
#include "logging/profile_logger.hpp"

namespace m3bp {

ExecutionContext::ExecutionContext()
	: m_configuration(make_unique<Configuration>())
	, m_logical_graph(make_unique<LogicalGraph>())
	, m_locality_manager(make_unique<LocalityManager>(*m_configuration))
	, m_scheduler(make_unique<Scheduler>(*m_locality_manager))
	, m_memory_manager(
		std::make_shared<MemoryManager>(m_configuration->affinity()))
	, m_profile_logger()
{
	const auto profile_destination = m_configuration->profile_log();
	if(profile_destination != ""){
		m_profile_logger =
			make_unique<ProfileLogger>(*m_configuration, *m_logical_graph);
	}
}

ExecutionContext::ExecutionContext(const Configuration &config)
	: m_configuration(make_unique<Configuration>(config))
	, m_logical_graph(make_unique<LogicalGraph>())
	, m_locality_manager(make_unique<LocalityManager>(*m_configuration))
	, m_scheduler(make_unique<Scheduler>(*m_locality_manager))
	, m_memory_manager(
		std::make_shared<MemoryManager>(m_configuration->affinity()))
	, m_profile_logger()
{
	const auto profile_destination = m_configuration->profile_log();
	if(profile_destination != ""){
		m_profile_logger =
			make_unique<ProfileLogger>(*m_configuration, *m_logical_graph);
	}
}

ExecutionContext::ExecutionContext(
	const Configuration &config,
	const FlowGraph &flow_graph)
	: m_configuration(make_unique<Configuration>(config))
	, m_logical_graph(make_unique<LogicalGraph>(
		build_logical_graph(flow_graph, *m_configuration)))
	, m_locality_manager(make_unique<LocalityManager>(*m_configuration))
	, m_scheduler(make_unique<Scheduler>(*m_locality_manager))
	, m_memory_manager(
		std::make_shared<MemoryManager>(m_configuration->affinity()))
	, m_profile_logger()
{
	const auto profile_destination = m_configuration->profile_log();
	if(profile_destination != ""){
		m_profile_logger =
			make_unique<ProfileLogger>(*m_configuration, *m_logical_graph);
	}
}

}

