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
#include <memory>
#include "m3bp/flow_graph.hpp"
#include "api/internal/flow_graph_impl.hpp"

namespace m3bp {

FlowGraph::FlowGraph()
	: m_impl(new internal::FlowGraphImpl())
{ }

FlowGraph::FlowGraph(const FlowGraph &graph)
	: m_impl(new internal::FlowGraphImpl(*graph.m_impl))
{ }

FlowGraph::FlowGraph(FlowGraph &&graph)
	: m_impl(std::move(graph.m_impl))
{ }

FlowGraph::~FlowGraph() = default;


FlowGraph &FlowGraph::operator=(const FlowGraph &graph){
	m_impl.reset(new internal::FlowGraphImpl(*graph.m_impl));
	return *this;
}

FlowGraph &FlowGraph::operator=(FlowGraph &&graph){
	m_impl = std::move(graph.m_impl);
	return *this;
}

VertexDescriptor FlowGraph::add_vertex(
	const std::string &name, internal::ProcessorWrapper &&proc)
{
	return m_impl->add_vertex(name, std::move(proc));
}

FlowGraph &FlowGraph::add_edge(
	OutputPortDescriptor producer,
	InputPortDescriptor  consumer)
{
	m_impl->add_edge(producer, consumer);
	return *this;
}

}

