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
#include <numeric>
#include "m3bp/configuration.hpp"
#include "scheduler/locality_manager.hpp"
#include "common/random.hpp"

namespace m3bp {

LocalityManager::LocalityManager()
	: m_max_concurrency(1)
	, m_numa_node_count(1)
	, m_thread_mapping()
	, m_partition_mapping()
	, m_node_to_worker()
{ }

LocalityManager::LocalityManager(const Configuration &config)
	: m_max_concurrency(config.max_concurrency())
	, m_numa_node_count(1)
	, m_thread_mapping()
	, m_partition_mapping()
	, m_node_to_worker()
{
	if(config.affinity() == AffinityMode::NONE){ return; }
	const auto num_threads = config.max_concurrency();
	const auto num_partitions = config.partition_count();
	auto &topo = Topology::instance();
	m_numa_node_count = topo.numa_node_count();
	m_node_to_worker.assign(m_numa_node_count, std::vector<identifier_type>());
	std::vector<size_type> pu_per_node(m_numa_node_count);
	for(identifier_type i = 0; i < m_numa_node_count; ++i){
		pu_per_node[i] = topo.processing_unit_per_node(i);
	}
	if(config.affinity() == AffinityMode::COMPACT){
		identifier_type node = 0;
		for(identifier_type i = 0, j = 0; i < num_threads; ++i){
			while(j == pu_per_node[node]){
				j = 0;
				node = (node + 1) % m_numa_node_count;
			}
			m_thread_mapping.push_back(node);
			m_node_to_worker[node].push_back(i);
			++j;
		}
	}else if(config.affinity() == AffinityMode::SCATTER){
		std::vector<size_type> remains(pu_per_node);
		const size_type total_pu_count =
			std::accumulate(pu_per_node.begin(), pu_per_node.end(), 0);
		identifier_type node = 0;
		for(identifier_type i = 0; i < num_threads; ++i){
			if(i % total_pu_count == 0){
				remains = pu_per_node;
				node = 0;
			}
			while(remains[node] == 0){
				node = (node + 1) % m_numa_node_count;
			}
			m_thread_mapping.push_back(node);
			m_node_to_worker[node].push_back(i);
			--remains[node];
			node = (node + 1) % m_numa_node_count;
		}
	}
	for(identifier_type i = 0; i < num_partitions; ++i){
		m_partition_mapping.push_back(
			m_thread_mapping[i % num_threads]);
	}
}


size_type LocalityManager::max_concurrency() const noexcept {
	return m_max_concurrency;
}

size_type LocalityManager::node_count() const noexcept {
	return m_numa_node_count;
}

identifier_type LocalityManager::thread_mapping(
	identifier_type worker_id) const noexcept
{
	if(m_thread_mapping.empty()){ return 0; }
	assert(worker_id < m_thread_mapping.size());
	return m_thread_mapping[worker_id];
}

identifier_type LocalityManager::partition_mapping(
	identifier_type partition_id) const noexcept
{
	if(m_partition_mapping.empty()){ return 0; }
	assert(partition_id < m_partition_mapping.size());
	return m_partition_mapping[partition_id];
}


void LocalityManager::set_thread_cpubind(identifier_type worker_id){
	if(m_thread_mapping.empty()){ return; }
	Topology::instance().set_thread_cpubind(thread_mapping(worker_id));
}


identifier_type LocalityManager::random_worker_from_node(identifier_type node){
	if(m_node_to_worker.empty()){
		return uniform_random_integer<identifier_type>(
			0, m_max_concurrency - 1);
	}
	const auto i = uniform_random_integer<identifier_type>(
		0, m_node_to_worker[node].size() - 1);
	return m_node_to_worker[node][i];
}

}

