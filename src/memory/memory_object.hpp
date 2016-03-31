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
#ifndef M3BP_MEMORY_MEMORY_OBJECT_HPP
#define M3BP_MEMORY_MEMORY_OBJECT_HPP

#include <memory>
#include <atomic>
#include <limits>
#include <boost/noncopyable.hpp>
#include "m3bp/types.hpp"

namespace m3bp {

class MemoryManager;

class MemoryObject : private boost::noncopyable {

private:
	static const identifier_type INVALID_IDENTIFIER =
		std::numeric_limits<identifier_type>::max();

	std::shared_ptr<MemoryManager> m_memory_manager;
	std::atomic<size_type> m_lock_count;
	identifier_type m_self_identifier;
	size_type m_buffer_size;
	identifier_type m_locality;
	void *m_pointer;

public:
	MemoryObject();
	MemoryObject(
		std::shared_ptr<MemoryManager> memory_manager,
		identifier_type identifier,
		size_type size);
	MemoryObject(
		std::shared_ptr<MemoryManager> memory_manager,
		identifier_type identifier,
		size_type size,
		identifier_type numa_node);
	~MemoryObject();

	identifier_type identifier() const;
	size_type size() const noexcept;
	identifier_type locality() const noexcept;

	void lock();
	void unlock() noexcept;

	const void *pointer() const;
	void *pointer();

};

}

#endif

