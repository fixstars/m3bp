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
#ifndef M3BP_CONTEXT_EXECUTOR_HPP
#define M3BP_CONTEXT_EXECUTOR_HPP

#include <memory>
#include <vector>
#include <thread>
#include <exception>
#include "m3bp/configuration.hpp"
#include "m3bp/flow_graph.hpp"

namespace m3bp {

class ThreadObserverBase;

class Executor {

public:
	using ThreadObserverPtr = std::shared_ptr<ThreadObserverBase>;

private:
	Configuration m_configuration;
	FlowGraph m_flow_graph;

	std::vector<ThreadObserverPtr> m_thread_observers;

	std::thread m_executor_thread;

	std::exception_ptr m_thrown_exception;

	std::string m_profile_destination;

public:
	Executor();

	void set_flow_graph(const FlowGraph &graph);
	void set_configuration(const Configuration &config);
	void add_thread_observer(ThreadObserverPtr observer);

	void execute();
	void wait();

};

}

#endif

