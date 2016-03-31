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
#ifndef M3BP_SCHEDULER_LOCALITY_MANAGER_HPP
#define M3BP_SCHEDULER_LOCALITY_MANAGER_HPP

#include "system/topology.hpp"

namespace m3bp {

class Configuration;

class LocalityManager {

private:
	size_type m_max_concurrency;
	size_type m_numa_node_count;
	std::vector<identifier_type> m_thread_mapping;
	std::vector<identifier_type> m_partition_mapping;
	std::vector<std::vector<identifier_type>> m_node_to_worker;

public:
	LocalityManager();
	LocalityManager(const Configuration &config);

	size_type max_concurrency() const noexcept;
	size_type node_count() const noexcept;

	identifier_type thread_mapping(
		identifier_type worker_id) const noexcept;

	identifier_type partition_mapping(
		identifier_type partition_id) const noexcept;

	void set_thread_cpubind(identifier_type worker_id);

	identifier_type random_worker_from_node(identifier_type node);

};

}

#endif

