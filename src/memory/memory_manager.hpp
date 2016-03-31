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
#ifndef M3BP_MEMORY_MEMORY_MANAGER_HPP
#define M3BP_MEMORY_MEMORY_MANAGER_HPP

#include <memory>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include "m3bp/types.hpp"
#include "m3bp/configuration.hpp"

namespace m3bp {

class MemoryObject;
class MemoryReference;

class MemoryManager
	: public std::enable_shared_from_this<MemoryManager>
	, private boost::noncopyable
{

private:
	using MemoryObjectWeakPtr = std::weak_ptr<MemoryObject>;

	AffinityMode m_affinity_mode;

	std::mutex m_mutex;
	std::unordered_map<identifier_type, MemoryObjectWeakPtr> m_managed_objects;
	std::atomic<identifier_type> m_next_identifier;

	std::atomic<size_type> m_total_memory_usage;
	bool m_assert_on_release;

public:
	MemoryManager();
	MemoryManager(AffinityMode affinity_mode);
	~MemoryManager();

	MemoryReference allocate(size_type size);
	MemoryReference allocate(size_type size, identifier_type numa_node);
	void notify_release(identifier_type identifier, size_type size) noexcept;

	size_type total_memory_usage() const;
	void log_memory_leaks();


};

}

#endif

