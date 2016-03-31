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
#include <utility>
#include "m3bp/context.hpp"
#include "m3bp/flow_graph.hpp"
#include "api/internal/context_impl.hpp"

namespace m3bp {

Context::Context()
	: m_impl(new internal::ContextImpl())
{ }

Context::Context(Context &&) noexcept = default;

Context::~Context() = default;


Context &Context::operator=(Context &&context) noexcept = default;


Context &Context::set_flow_graph(const FlowGraph &graph){
	m_impl->set_flow_graph(graph);
	return *this;
}


Context &Context::set_configuration(const Configuration &config){
	m_impl->set_configuration(config);
	return *this;
}


Context &Context::add_thread_observer(std::shared_ptr<ThreadObserverBase> observer){
	m_impl->add_thread_observer(observer);
	return *this;
}


void Context::execute(){
	m_impl->execute();
}

void Context::wait(){
	m_impl->wait();
}

}

