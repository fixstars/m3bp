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
#ifndef M3BP_API_INTERNAL_CONTEXT_IMPL_HPP
#define M3BP_API_INTERNAL_CONTEXT_IMPL_HPP

#include "m3bp/configuration.hpp"
#include "m3bp/exception.hpp"
#include "context/executor.hpp"
#include "api/internal/flow_graph_impl.hpp"

namespace m3bp {
namespace internal {

class ContextImpl {

private:
	Executor m_executor;

public:
	ContextImpl()
		: m_executor()
	{ }


	ContextImpl &set_flow_graph(const FlowGraph &graph){
		if(FlowGraphImpl::get_impl(graph).has_cycle()){
			throw m3bp::RoutingError("A cycle was detected");
		}
		m_executor.set_flow_graph(graph);
		return *this;
	}


	ContextImpl &set_configuration(const Configuration &config){
		m_executor.set_configuration(config);
		return *this;
	}


	void add_thread_observer(std::shared_ptr<ThreadObserverBase> observer){
		m_executor.add_thread_observer(std::move(observer));
	}


	void execute(){
		m_executor.execute();
	}

	void wait(){
		m_executor.wait();
	}

};

}
}

#endif

