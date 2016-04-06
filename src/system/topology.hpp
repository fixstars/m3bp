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
#ifndef M3BP_SYSTEM_TOPOLOGY_HPP
#define M3BP_SYSTEM_TOPOLOGY_HPP

#include <cassert>
#include <vector>

#ifdef M3BP_LOCALITY_ENABLED
#include <hwloc.h>
#endif

#include "m3bp/types.hpp"

namespace m3bp {

class Topology {

private:
#ifdef M3BP_LOCALITY_ENABLED
	hwloc_topology_t m_topology;
#endif

	std::vector<identifier_type> m_available_processing_units;
	std::vector<identifier_type> m_available_numa_nodes;
	std::vector<size_type> m_processing_units_per_node;

	Topology();
	Topology(const Topology &) = delete;
	Topology &operator=(const Topology &) = delete;

public:
	~Topology();

	static Topology &instance();

	size_type total_processing_unit_count() const noexcept {
		return m_available_processing_units.size();
	}
	size_type numa_node_count() const noexcept {
		return m_available_numa_nodes.size();
	}

	size_type processing_unit_per_node(
		identifier_type numa_node) const noexcept
	{
		assert(numa_node < numa_node_count());
		return m_processing_units_per_node[numa_node];
	}

	void set_thread_cpubind(identifier_type numa_node);

	void *allocate_membind(size_type size);
	void *allocate_membind(size_type size, identifier_type numa_node);

	void release_membind(void *p, size_type size) noexcept;

};

}

#endif

