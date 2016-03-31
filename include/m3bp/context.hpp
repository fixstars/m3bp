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
#ifndef M3BP_CONTEXT_HPP
#define M3BP_CONTEXT_HPP

#include <memory>
#include "m3bp/thread_observer_base.hpp"

namespace m3bp {

namespace internal {

class ContextImpl;

}


class FlowGraph;
class Configuration;

/**
 *  A context for managing executions.
 */
class Context {

public:
	/**
	 *  Initializes this context by default settings.
	 */
	Context();

	Context(const Context &) = delete;

	Context(Context &&context) noexcept;

	~Context();


	Context &operator=(const Context &) = delete;

	Context &operator=(Context &&context) noexcept;


	/**
	 *  Sets a flow graph that describes work flow.
	 *
	 *  @param[in] graph  A flow graph that describes tasks and dependencies
	 *                    of them.
	 */
	Context &set_flow_graph(const FlowGraph &graph);


	/**
	 *  Sets configration values.
	 *
	 *  @param[in] config  A configuration object that has a set of
	 *                     configuration values.
	 *  @return    The reference to this context.
	 */
	Context &set_configuration(const Configuration &config);


	/**
	 *  Adds an observer of worker threads.
	 *
	 *  @param  observer  An instance of observer that will be added.
	 *  @return The reference to this context.
	 */
	Context &add_thread_observer(std::shared_ptr<ThreadObserverBase> observer);


	/**
	 *  Processes all tasks in a flow graph.
	 */
	void execute();

	/**
	 *  Waits for the last execution to be over.
	 */
	void wait();


private:
	friend class internal::ContextImpl;
	std::unique_ptr<internal::ContextImpl> m_impl;

};

}

#endif

