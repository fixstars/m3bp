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
#include <cassert>
#include "memory/memory_manager.hpp"
#include "memory/memory_object.hpp"
#include "memory/memory_reference.hpp"
#include "logging/general_logger.hpp"
#include "logging/profile_logger.hpp"
#include "logging/profile_event_logger.hpp"

#define M3BP_MEMORY_MANAGER_TRACE \
	M3BP_GENERAL_LOG(TRACE) << "[MemoryManager] [" << __func__ << "] "

namespace m3bp {

MemoryManager::MemoryManager()
	: std::enable_shared_from_this<MemoryManager>()
	, m_affinity_mode(AffinityMode::NONE)
	, m_mutex()
	, m_managed_objects()
	, m_next_identifier(0)
	, m_total_memory_usage(0)
	, m_assert_on_release(false)
{ }

MemoryManager::MemoryManager(AffinityMode affinity)
	: std::enable_shared_from_this<MemoryManager>()
	, m_affinity_mode(affinity)
	, m_mutex()
	, m_managed_objects()
	, m_next_identifier(0)
	, m_total_memory_usage(0)
	, m_assert_on_release(false)
{ }

MemoryManager::~MemoryManager() = default;


MemoryReference MemoryManager::allocate(size_type size){
	const auto new_id = m_next_identifier++;
	M3BP_MEMORY_MANAGER_TRACE << size << " " << new_id;
	auto mobj = std::make_shared<MemoryObject>(
		shared_from_this(), new_id, size);
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_managed_objects.emplace(new_id, mobj);
	}
	m_total_memory_usage += size;
	auto &event_logger = ProfileLogger::thread_local_logger();
	event_logger.log_allocate_memory(mobj->identifier(), size);
	return MemoryReference(std::move(mobj));
}

MemoryReference MemoryManager::allocate(
	size_type size, identifier_type numa_node)
{
	if(m_affinity_mode == AffinityMode::NONE){
		return allocate(size);
	}
	const auto new_id = m_next_identifier++;
	M3BP_MEMORY_MANAGER_TRACE << size << " " << new_id << " " << numa_node;
	auto mobj = std::make_shared<MemoryObject>(
		shared_from_this(), new_id, size, numa_node);
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_managed_objects.emplace(new_id, mobj);
	}
	m_total_memory_usage += size;
	auto &event_logger = ProfileLogger::thread_local_logger();
	event_logger.log_allocate_memory(mobj->identifier(), size, numa_node);
	return MemoryReference(std::move(mobj));
}

void MemoryManager::notify_release(
	identifier_type identifier,
	size_type size) noexcept
{
	M3BP_MEMORY_MANAGER_TRACE << identifier;
	auto &event_logger = ProfileLogger::thread_local_logger();
	event_logger.log_release_memory(identifier);
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_managed_objects.find(identifier);
	assert(it != m_managed_objects.end());
	m_managed_objects.erase(it);
	m_total_memory_usage -= size;
	assert(!m_assert_on_release);
}


size_type MemoryManager::total_memory_usage() const {
	return m_total_memory_usage.load();
}

void MemoryManager::log_memory_leaks(){
	for(const auto &p : m_managed_objects){
		if(p.second.expired()){
			M3BP_GENERAL_LOG(WARNING)
				<< "Detected memory leaks: Memory object #" << p.first
				<< " (expired)";
		}else{
			M3BP_GENERAL_LOG(WARNING)
				<< "Detected memory leaks: Memory object #" << p.first;
		}
	}
	m_assert_on_release = true;
}

}

