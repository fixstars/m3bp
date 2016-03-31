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

TEST(Topology, SystemInfo){
	auto &topo = m3bp::Topology::instance();
	const auto num_pu = topo.total_processing_unit_count();
	EXPECT_EQ(std::thread::hardware_concurrency(), num_pu);
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

