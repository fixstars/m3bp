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
#ifndef M3BP_CONTEXT_EXECUTION_CONTEXT_HPP
#define M3BP_CONTEXT_EXECUTION_CONTEXT_HPP

#include <memory>
#include <cassert>
#include "m3bp/configuration.hpp"
#include "graph/logical_graph.hpp"
#include "scheduler/locality_manager.hpp"
#include "scheduler/scheduler.hpp"
#include "memory/memory_manager.hpp"
#include "logging/profile_logger.hpp"

namespace m3bp {

class FlowGraph;

class ExecutionContext {

private:
	std::unique_ptr<Configuration>   m_configuration;
	std::unique_ptr<LogicalGraph>    m_logical_graph;
	std::unique_ptr<LocalityManager> m_locality_manager;
	std::unique_ptr<Scheduler>       m_scheduler;
	std::shared_ptr<MemoryManager>   m_memory_manager;
	std::unique_ptr<ProfileLogger>   m_profile_logger;

public:
	ExecutionContext();
	explicit ExecutionContext(const Configuration &config);
	ExecutionContext(
		const Configuration &config,
		const FlowGraph &flow_graph);

	const Configuration &configuration() const noexcept {
		return *m_configuration;
	}

	const LogicalGraph &logical_graph() const noexcept {
		return *m_logical_graph;
	}
	LogicalGraph &logical_graph() noexcept {
		return *m_logical_graph;
	}

	const LocalityManager &locality_manager() const noexcept {
		return *m_locality_manager;
	}
	LocalityManager &locality_manager() noexcept {
		return *m_locality_manager;
	}

	const Scheduler &scheduler() const noexcept {
		return *m_scheduler;
	}
	Scheduler &scheduler() noexcept {
		return *m_scheduler;
	}

	const MemoryManager &memory_manager() const noexcept {
		return *m_memory_manager;
	}
	MemoryManager &memory_manager() noexcept {
		return *m_memory_manager;
	}

	bool is_profile_enabled() const noexcept {
		return m_profile_logger != nullptr;
	}
	const ProfileLogger &profile_logger() const noexcept {
		assert(m_profile_logger);
		return *m_profile_logger;
	}
	ProfileLogger &profile_logger() noexcept {
		assert(m_profile_logger);
		return *m_profile_logger;
	}

};

}

#endif

