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
#ifndef M3BP_API_INTERNAL_PROCESSOR_BASE_IMPL_HPP
#define M3BP_API_INTERNAL_PROCESSOR_BASE_IMPL_HPP

#include <limits>
#include "m3bp/exception.hpp"
#include "m3bp/input_port.hpp"
#include "m3bp/output_port.hpp"

namespace m3bp {
namespace internal {

class ProcessorBaseImpl {

private:
	std::vector<InputPort>  m_input_ports;
	std::vector<OutputPort> m_output_ports;

	size_type m_task_count;
	size_type m_max_concurrency;

public:
	ProcessorBaseImpl()
		: m_input_ports()
		, m_output_ports()
		, m_task_count(0)
		, m_max_concurrency(std::numeric_limits<size_type>::max())
	{ }

	ProcessorBaseImpl(
		const std::vector<InputPort>  &input_ports,
		const std::vector<OutputPort> &output_ports)
		: m_input_ports(input_ports)
		, m_output_ports(output_ports)
		, m_task_count(0)
		, m_max_concurrency(std::numeric_limits<size_type>::max())
	{
		bool has_one_to_one = false, has_scatter_gather = false;
		for(const auto &ip : input_ports){
			if(ip.movement() == Movement::ONE_TO_ONE){
				has_one_to_one = true;
			}else if(ip.movement() == Movement::SCATTER_GATHER){
				has_scatter_gather = true;
			}
		}
		if(has_one_to_one && has_scatter_gather){
			throw ProcessorDefinitionError(
				"A processor cannot have both an ONE_TO_ONE port and "
				"a SCATTER_GATHER pors");
		}
	}


	const std::vector<InputPort> &input_ports() const {
		return m_input_ports;
	}

	const std::vector<OutputPort> &output_ports() const {
		return m_output_ports;
	}


	size_type task_count() const noexcept {
		return m_task_count;
	}

	ProcessorBaseImpl &task_count(size_type new_count){
		m_task_count = new_count;
		return *this;
	}


	size_type max_concurrency() const noexcept {
		return m_max_concurrency;
	}

	ProcessorBaseImpl &max_concurrency(size_type new_concurrency){
		m_max_concurrency = new_concurrency;
		return *this;
	}

};

}
}

#endif

