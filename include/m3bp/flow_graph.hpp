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
#ifndef M3BP_FLOW_GRAPH_HPP
#define M3BP_FLOW_GRAPH_HPP

#include <limits>
#include <memory>
#include "m3bp/types.hpp"
#include "m3bp/processor_base.hpp"
#include "m3bp/internal/processor_wrapper.hpp"

namespace m3bp {

namespace internal {

class FlowGraphImpl;

}


class PortDescriptorBase {

public:
	/// Identifier that means an invalid vertex/port.
	static const identifier_type INVALID_ID =
		std::numeric_limits<identifier_type>::max();

private:
	identifier_type m_vertex_id;
	identifier_type m_port_id;

protected:
	PortDescriptorBase()
		: m_vertex_id(INVALID_ID)
		, m_port_id(INVALID_ID)
	{ }

	PortDescriptorBase(
		identifier_type vertex_id,
		identifier_type port_id)
		: m_vertex_id(vertex_id)
		, m_port_id(port_id)
	{ }

public:
	identifier_type vertex_id() const {
		return m_vertex_id;
	}

	identifier_type port_id() const {
		return m_port_id;
	}

	bool is_valid() const {
		return m_vertex_id != INVALID_ID && m_port_id != INVALID_ID;
	}

};

/**
 *  This class describes a descriptor of an input port.
 */
class InputPortDescriptor : public PortDescriptorBase {

public:
	/**
	 *  Constructs an invalid descriptor.
	 */
	InputPortDescriptor()
		: PortDescriptorBase()
	{ }

	/**
	 *  Constructs a descriptor from identifiers.
	 */
	InputPortDescriptor(
		identifier_type vertex_id,
		identifier_type port_id)
		: PortDescriptorBase(vertex_id, port_id)
	{ }

};

/**
 *  This class describes a descriptor of an output port.
 */
class OutputPortDescriptor : public PortDescriptorBase {

public:
	/**
	 *  Constructs an invalid descriptor.
	 */
	OutputPortDescriptor()
		: PortDescriptorBase()
	{ }

	/**
	 *  Constructs a descriptor from identifiers.
	 */
	OutputPortDescriptor(
		identifier_type vertex_id,
		identifier_type port_id)
		: PortDescriptorBase(vertex_id, port_id)
	{ }

};


/**
 *  This class describes a descriptor of a vertex.
 */
class VertexDescriptor {

public:
	/// Identifier that means an invalid vertex/port.
	static const identifier_type INVALID_ID =
		std::numeric_limits<identifier_type>::max();

private:
	identifier_type m_vertex_id;
	size_type m_input_port_count;
	size_type m_output_port_count;

public:
	/**
	 *  Constructs an invalid vertex descriptor.
	 */
	VertexDescriptor()
		: m_vertex_id(INVALID_ID)
		, m_input_port_count(0)
		, m_output_port_count(0)
	{ }

	/**
	 *  Constructs a vertex descriptor.
	 *
	 *  @param vertex_id  An identifier of the vertex which this descriptor
	 *                    refers.
	 *  @param proc       A processor of the vertex which this descriptor
	 *                    refers.
	 */
	VertexDescriptor(identifier_type vertex_id, const ProcessorBase &proc)
		: m_vertex_id(vertex_id)
		, m_input_port_count(proc.input_ports().size())
		, m_output_port_count(proc.output_ports().size())
	{ }

	/**
	 *  Gets a descriptor of an input port.
	 *
	 *  @param[in] port_id  An index of a port to get a descriptor.
	 *  @return    The descriptor of the specified port.
	 */
	InputPortDescriptor input_port(identifier_type port_id) const {
		if(port_id >= m_input_port_count){
			throw std::out_of_range("too large port_id");
		}
		return InputPortDescriptor(m_vertex_id, port_id);
	}

	/**
	 *  Gets a descriptor of an output port.
	 *
	 *  @param[in] port_id  An index of a port to get a descriptor.
	 *  @return    The descriptor of the specified port.
	 */
	OutputPortDescriptor output_port(identifier_type port_id) const {
		if(port_id >= m_output_port_count){
			throw std::out_of_range("too large port_id");
		}
		return OutputPortDescriptor(m_vertex_id, port_id);
	}

};

/**
 *  A flow graph that describes a workload.
 */
class FlowGraph {

public:
	/**
	 *  Constructs an empty flow graph.
	 */
	FlowGraph();

	/**
	 *  Copy constructor.
	 */
	FlowGraph(const FlowGraph &graph);

	/**
	 *  Move constructor.
	 */
	FlowGraph(FlowGraph &&graph);

	/**
	 *  Destructor.
	 */
	~FlowGraph();


	/**
	 *  Copy substitution operator.
	 */
	FlowGraph &operator=(const FlowGraph &graph);

	/**
	 *  Move substitution operator.
	 */
	FlowGraph &operator=(FlowGraph &&graph);


	/**
	 *  Adds a vertex to the flow graph.
	 *
	 *  @param  name  A name of a vertex to be added.
	 *  @param  proc  A processor of a vertex to be added.
	 *  @return A descriptor of the vertex that is added.
	 */
	template <typename ProcessorType>
	VertexDescriptor add_vertex(
		const std::string &name, const ProcessorType &proc)
	{
		return add_vertex(name, internal::ProcessorWrapper(proc));
	}

	/**
	 *  Adds an edge to the flow graph.
	 *
	 *  @param  producer  An initial port of an edge to be added.
	 *  @param  consumer  An terminal port of an edge to be added.
	 *  @return A reference of this flow graph.
	 */
	FlowGraph &add_edge(
		OutputPortDescriptor producer,
		InputPortDescriptor  consumer);

private:
	VertexDescriptor add_vertex(
		const std::string &name, internal::ProcessorWrapper &&proc);

	friend class internal::FlowGraphImpl;
	std::unique_ptr<internal::FlowGraphImpl> m_impl;

};

}

#endif

