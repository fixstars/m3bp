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
#ifndef M3BP_API_INTERNAL_FLOW_GRAPH_IMPL_HPP
#define M3BP_API_INTERNAL_FLOW_GRAPH_IMPL_HPP

#include "common/array_ref.hpp"
#include "api/internal/vertex.hpp"

namespace m3bp {
namespace internal {

class FlowGraphImpl {

private:
	std::vector<Vertex> m_vertices;

	bool is_valid_descriptor(
		const InputPortDescriptor &iport) const noexcept
	{
		if(iport.vertex_id() >= m_vertices.size()){ return false; }
		const auto &ports = m_vertices[iport.vertex_id()].input_ports();
		return iport.port_id() < static_cast<size_type>(ports.size());
	}

	bool is_valid_descriptor(
		const OutputPortDescriptor &oport) const noexcept
	{
		if(oport.vertex_id() >= m_vertices.size()){ return false; }
		const auto &ports = m_vertices[oport.vertex_id()].output_ports();
		return oport.port_id() < static_cast<size_type>(ports.size());
	}

	bool has_cycle_recur(
		std::vector<bool> &done,
		std::vector<bool> &in_stack,
		identifier_type u) const
	{
		if(in_stack[u]){ return true; }
		if(done[u]){ return false; }
		done[u] = true;
		in_stack[u] = true;
		const auto &vertex = m_vertices[u];
		for(const auto &sources : vertex.sources()){
			for(const auto &src : sources){
				if(has_cycle_recur(done, in_stack, src.vertex_id())){
					return true;
				}
			}
		}
		in_stack[u] = false;
		return false;
	}

public:
	VertexDescriptor add_vertex(
		const std::string &name, ProcessorWrapper &&proc)
	{
		const VertexDescriptor desc(m_vertices.size(), *proc);
		m_vertices.emplace_back(name, std::move(proc));
		return desc;
	}

	FlowGraphImpl &add_edge(
		OutputPortDescriptor producer,
		InputPortDescriptor  consumer)
	{
		// test validity of descriptors
		if(!is_valid_descriptor(producer)){
			throw std::invalid_argument("invalid descriptor");
		}
		if(!is_valid_descriptor(consumer)){
			throw std::invalid_argument("invalid descriptor");
		}
		// test connectivity of producer with consumer
		const auto &producer_vertex = m_vertices[producer.vertex_id()];
		const auto &oport = producer_vertex.output_ports()[producer.port_id()];
		auto &consumer_vertex = m_vertices[consumer.vertex_id()];
		const auto &iport = consumer_vertex.input_ports()[consumer.port_id()];
		if(iport.movement() == Movement::SCATTER_GATHER && !oport.has_key()){
			// producer of scatter-gather port must have keys
			throw std::invalid_argument("mismatched port specifications");
		}
		consumer_vertex.sources()[consumer.port_id()].push_back(producer);
		return *this;
	}


	ArrayRef<const Vertex> vertices() const {
		const auto ptr = m_vertices.data();
		return ArrayRef<const Vertex>(ptr, ptr + m_vertices.size());
	}

	ArrayRef<Vertex> vertices(){
		const auto ptr = m_vertices.data();
		return ArrayRef<Vertex>(ptr, ptr + m_vertices.size());
	}


	bool has_cycle() const {
		const auto n = m_vertices.size();
		std::vector<bool> done(n), in_stack(n);
		for(identifier_type i = 0; i < n; ++i){
			if(done[i]){ continue; }
			if(has_cycle_recur(done, in_stack, i)){ return true; }
		}
		return false;
	}


	static const FlowGraphImpl &get_impl(const FlowGraph &graph){
		return *graph.m_impl;
	}

};

}
}

#endif

