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
#include <gtest/gtest.h>
#include <thread>
#include "system/topology.hpp"

#ifdef M3BP_LOCALITY_ENABLED
#include <hwloc.h>
#endif

namespace {

#ifdef M3BP_LOCALITY_ENABLED
m3bp::size_type available_processor_count(){
	hwloc_topology_t topology;
	EXPECT_EQ(0, hwloc_topology_init(&topology));
	EXPECT_EQ(0, hwloc_topology_load(topology));

	const auto n = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_PU);
	EXPECT_GT(n, 1);

	hwloc_cpuset_t before_cpuset = hwloc_bitmap_alloc();
	hwloc_get_cpubind(topology, before_cpuset, HWLOC_CPUBIND_THREAD);

	m3bp::size_type count = 0;
	for(int i = 0; i < n; ++i){
		const auto obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_PU, i);
		if(hwloc_set_cpubind(topology, obj->cpuset, HWLOC_CPUBIND_THREAD) == 0){
			++count;
		}
	}

	hwloc_set_cpubind(topology, before_cpuset, HWLOC_CPUBIND_THREAD);
	hwloc_bitmap_free(before_cpuset);
	hwloc_topology_destroy(topology);
	return count;
}
#else
m3bp::size_type available_processor_count(){
	return std::thread::hardware_concurrency();
}
#endif

}

TEST(Topology, SystemInfo){
	auto &topo = m3bp::Topology::instance();
	const auto num_pu = topo.total_processing_unit_count();
	EXPECT_EQ(available_processor_count(), num_pu);
	EXPECT_GE(num_pu, 1);
	const auto num_nodes = topo.numa_node_count();
	m3bp::size_type sum = 0;
	for(m3bp::identifier_type i = 0; i < num_nodes; ++i){
		sum += topo.processing_unit_per_node(i);
		topo.set_thread_cpubind(i);
	}
	EXPECT_EQ(num_pu, sum);
}

TEST(Topology, MemoryBind){
	const auto n = 10000;
	auto &topo = m3bp::Topology::instance();
	const auto num_nodes = topo.numa_node_count();
	for(m3bp::identifier_type i = 0; i < num_nodes; ++i){
		auto p = reinterpret_cast<uint8_t *>(topo.allocate_membind(n, i));
		std::fill(p, p + n, 0xcc);
		topo.release_membind(p, n);
	}
}

