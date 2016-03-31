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
#include <cassert>
#include "memory/memory_object.hpp"
#include "memory/memory_manager.hpp"
#include "system/topology.hpp"

namespace m3bp {

MemoryObject::MemoryObject()
	: m_memory_manager()
	, m_lock_count(0)
	, m_self_identifier(INVALID_IDENTIFIER)
	, m_buffer_size(0)
	, m_locality(0)
	, m_pointer(nullptr)
{ }

MemoryObject::MemoryObject(
	std::shared_ptr<MemoryManager> memory_manager,
	identifier_type identifier,
	size_type size)
	: m_memory_manager(std::move(memory_manager))
	, m_lock_count(0)
	, m_self_identifier(identifier)
	, m_buffer_size(size)
	, m_locality(0)
	, m_pointer(nullptr)
{
	assert(m_memory_manager);
	auto &topo = Topology::instance();
	m_pointer = topo.allocate_membind(size);
}

MemoryObject::MemoryObject(
	std::shared_ptr<MemoryManager> memory_manager,
	identifier_type identifier,
	size_type size,
	identifier_type numa_node)
	: m_memory_manager(std::move(memory_manager))
	, m_lock_count(0)
	, m_self_identifier(identifier)
	, m_buffer_size(size)
	, m_locality(numa_node)
	, m_pointer(nullptr)
{
	assert(m_memory_manager);
	auto &topo = Topology::instance();
	m_pointer = topo.allocate_membind(size, numa_node);
}

MemoryObject::~MemoryObject(){
	assert(m_lock_count.load() == 0);
	if(m_self_identifier != INVALID_IDENTIFIER){
		auto &topo = Topology::instance();
		topo.release_membind(m_pointer, m_buffer_size);
		m_memory_manager->notify_release(m_self_identifier, m_buffer_size);
	}
}


identifier_type MemoryObject::identifier() const {
	return m_self_identifier;
}

size_type MemoryObject::size() const noexcept {
	return m_buffer_size;
}

identifier_type MemoryObject::locality() const noexcept {
	return m_locality;
}


void MemoryObject::lock(){
	m_lock_count++;
}

void MemoryObject::unlock() noexcept {
	assert(m_lock_count-- > 0);
}


const void *MemoryObject::pointer() const {
	assert(m_lock_count.load() > 0);
	return m_pointer;
}

void *MemoryObject::pointer(){
	assert(m_lock_count.load() > 0);
	return m_pointer;
}

}

