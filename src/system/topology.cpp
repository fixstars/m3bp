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
#include <stdexcept>
#include <thread>
#include <atomic>
#include "system/topology.hpp"
#include "logging/general_logger.hpp"

namespace m3bp {
namespace {

#ifdef M3BP_LOCALITY_ENABLED
thread_local identifier_type g_binded_node = 0;

hwloc_topology_t initialize_topology(){
	hwloc_topology_t topology;
	if(hwloc_topology_init(&topology) != 0){
		throw std::runtime_error(
			"An error occured on `hwloc_topology_init()`");
	}
	if(hwloc_topology_load(topology) != 0){
		throw std::runtime_error(
			"An error occured on `hwloc_topology_load()`");
	}
	return topology;
}

size_type count_processing_units(hwloc_topology_t topology){
	const auto retval = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_PU);
	if(retval == -1){
		throw std::runtime_error(
			"An error occured on `hwloc_get_nbobjs_by_type(HWLOC_OBJ_PU)`");
	}else if(retval == 0){
		return 1;
	}
	return retval;
}

size_type count_numa_nodes(hwloc_topology_t topology){
	if(hwloc_get_type_depth(topology, HWLOC_OBJ_NODE) <= 0){
		// Current system topology does not have the depth of NUMA nodes
		return 1;
	}
	const auto retval = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_NODE);
	if(retval == -1){
		throw std::runtime_error(
			"An error occured on `hwloc_get_nbobjs_by_type(HWLOC_OBJ_NODE)`");
	}else if(retval == 0){
		return 1;
	}
	return retval;
}
#endif

}

#ifdef M3BP_LOCALITY_ENABLED

Topology::Topology()
	: m_topology(initialize_topology())
	, m_total_processing_unit_count(count_processing_units(m_topology))
	, m_processing_units_per_node(count_numa_nodes(m_topology))
{
	const auto num_nodes = m_processing_units_per_node.size();
	for(identifier_type i = 0; i < m_total_processing_unit_count; ++i){
		if(num_nodes == 1){
			++m_processing_units_per_node[0];
		}else{
			auto obj = hwloc_get_obj_by_type(m_topology, HWLOC_OBJ_PU, i);
			while(obj && obj->type != HWLOC_OBJ_NODE){ obj = obj->parent; }
			if(obj && obj->type == HWLOC_OBJ_NODE){
				++m_processing_units_per_node[obj->logical_index];
			}else{
				++m_processing_units_per_node[0];
			}
		}
	}
}

Topology::~Topology(){
	hwloc_topology_destroy(m_topology);
}

#else

Topology::Topology()
	: m_total_processing_unit_count(std::thread::hardware_concurrency())
	, m_processing_units_per_node(1, m_total_processing_unit_count)
{ }

Topology::~Topology() = default;

#endif

Topology &Topology::instance(){
	static Topology s_instance;
	return s_instance;
}


void Topology::set_thread_cpubind(identifier_type numa_node){
#ifdef M3BP_LOCALITY_ENABLED
	const auto num_nodes = m_processing_units_per_node.size();
	assert(numa_node < num_nodes);
	hwloc_obj_t obj;
	if(num_nodes == 1){
		obj = hwloc_get_obj_by_type(m_topology, HWLOC_OBJ_MACHINE, 0);
		if(!obj){
			throw std::runtime_error(
				"An error occured on "
				"`hwloc_get_obj_by_type(HWLOC_OBJ_MACHINE)`");
		}
	}else{
		obj = hwloc_get_obj_by_type(m_topology, HWLOC_OBJ_NODE, numa_node);
		if(!obj){
			throw std::runtime_error(
				"An error occured on `hwloc_get_obj_by_type(HWLOC_OBJ_NODE)`");
		}
	}
	if(hwloc_set_cpubind(m_topology, obj->cpuset, HWLOC_CPUBIND_THREAD) != 0){
		throw std::runtime_error("An error occured on `hwloc_set_cpubind()`");
	}
	g_binded_node = numa_node;
#endif
}


void *Topology::allocate_membind(size_type size){
	return allocate_membind(size, g_binded_node);
}

void *Topology::allocate_membind(
	size_type size, identifier_type numa_node)
{
	(void)(numa_node);
	assert(numa_node < m_processing_units_per_node.size());
	void *ptr = malloc(size);
	if(!ptr){ throw std::bad_alloc(); }
	return ptr;
}

void Topology::release_membind(
	void *p, size_type /* size */) noexcept
{
	free(p);
}

}

