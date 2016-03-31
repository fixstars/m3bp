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
#ifndef M3BP_TASKS_SHUFFLE_SHUFFLE_BUFFER_HPP
#define M3BP_TASKS_SHUFFLE_SHUFFLE_BUFFER_HPP

#include <atomic>
#include <cassert>
#include "m3bp/types.hpp"
#include "common/make_unique.hpp"
#include "common/array_ref.hpp"
#include "memory/memory_reference.hpp"
#include "memory/memory_manager.hpp"

namespace m3bp {

class ShuffleBuffer {

public:

private:
	LockedMemoryReference m_memory_object;

	size_type m_partition_count;
	size_type *m_offsets;
	void *m_data;

public:
	ShuffleBuffer()
		: m_memory_object()
		, m_partition_count(0)
		, m_offsets()
		, m_data()
	{ }

	ShuffleBuffer(
		MemoryManager &memory_manager,
		size_type buffer_size,
		size_type partition_count,
		identifier_type locality)
		: m_memory_object()
		, m_partition_count(partition_count)
		, m_offsets()
		, m_data()
	{
		m_memory_object = memory_manager.allocate(
			sizeof(size_type) * (partition_count + 1) + buffer_size,
			locality).lock();
		const auto base_ptr =
			reinterpret_cast<uintptr_t>(m_memory_object.pointer());
		m_offsets = reinterpret_cast<size_type *>(base_ptr);
		m_data = reinterpret_cast<void *>(
			base_ptr + sizeof(size_type) * (partition_count + 1));
	}

	ShuffleBuffer(LockedMemoryReference mobj, size_type partition_count)
		: m_memory_object(std::move(mobj))
		, m_partition_count(partition_count)
		, m_offsets(nullptr)
		, m_data(nullptr)
	{
		const auto base_ptr =
			reinterpret_cast<uintptr_t>(m_memory_object.pointer());
		m_offsets = reinterpret_cast<size_type *>(base_ptr);
		m_data = reinterpret_cast<void *>(
			base_ptr + sizeof(size_type) * (partition_count + 1));
	}


	LockedMemoryReference raw_reference(){
		return m_memory_object;
	}


	ArrayRef<const size_type> offsets() const {
		return ArrayRef<const size_type>(
			m_offsets, m_offsets + m_partition_count + 1);
	}
	ArrayRef<size_type> offsets(){
		return ArrayRef<size_type>(
			m_offsets, m_offsets + m_partition_count + 1);
	}

	const void *data() const { return m_data; }
	void *data(){ return m_data; }

};

}

#endif

