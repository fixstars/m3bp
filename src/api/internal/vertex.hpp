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
#ifndef M3BP_API_INTERNAL_VERTEX_HPP
#define M3BP_API_INTERNAL_VERTEX_HPP

#include <string>
#include <boost/optional.hpp>
#include "m3bp/flow_graph.hpp"
#include "m3bp/processor_base.hpp"
#include "m3bp/internal/processor_wrapper.hpp"

namespace m3bp {
namespace internal {

class Vertex {

public:
	using SourcePorts = std::vector<OutputPortDescriptor>;

private:
	std::string m_name;
	ProcessorWrapper m_processor;

	std::vector<std::vector<OutputPortDescriptor>> m_sources;

public:
	Vertex() = default;

	Vertex(const std::string &name, const ProcessorWrapper &proc)
		: m_name(name)
		, m_processor(proc)
		, m_sources(m_processor->input_ports().size())
	{ }

	Vertex(const std::string &name, ProcessorWrapper &&proc)
		: m_name(name)
		, m_processor(std::move(proc))
		, m_sources(m_processor->input_ports().size())
	{ }


	const std::string &name() const {
		return m_name;
	}


	const ProcessorWrapper &processor() const {
		return m_processor;
	}

	ProcessorWrapper &processor(){
		return m_processor;
	}


	ArrayRef<const InputPort> input_ports() const {
		const auto &iports = m_processor->input_ports();
		return ArrayRef<const InputPort>(
			iports.data(), iports.data() + iports.size());
	}

	ArrayRef<const OutputPort> output_ports() const {
		const auto &oports = m_processor->output_ports();
		return ArrayRef<const OutputPort>(
			oports.data(), oports.data() + oports.size());
	}


	ArrayRef<const SourcePorts> sources() const {
		const auto ptr = m_sources.data();
		return ArrayRef<const SourcePorts>(ptr, ptr + m_sources.size());
	}

	ArrayRef<SourcePorts> sources(){
		const auto ptr = m_sources.data();
		return ArrayRef<SourcePorts>(ptr, ptr + m_sources.size());
	}

};

}
}

#endif

