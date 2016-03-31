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
#include <cstddef>
#include <cassert>
#include "memory/serialized_buffer.hpp"
#include "memory/memory_manager.hpp"

namespace m3bp {

namespace {

inline size_type align_ceil(size_type x, size_type align){
	assert((align & (align - 1)) == 0);
	return (x + align - 1) & ~(align - 1);
}

}

SerializedBuffer SerializedBuffer::allocate_value_only_buffer(
	MemoryManager &memory_manager,
	size_type maximum_record_count,
	size_type total_record_size,
	identifier_type target_node)
{
	size_type buffer_size = 0;
	// Common header
	const ptrdiff_t common_header_ptrdiff = buffer_size;
	buffer_size += sizeof(SerializedBufferHeader);
	// Values
	const ptrdiff_t values_header_ptrdiff = buffer_size;
	buffer_size += sizeof(SerializedValuesHeader);
	buffer_size  = align_ceil(buffer_size, alignof(max_align_t));
	const ptrdiff_t values_data_ptrdiff = buffer_size;
	buffer_size += align_ceil(total_record_size, alignof(size_type)); // data
	const ptrdiff_t values_offsets_ptrdiff = buffer_size;
	buffer_size += (maximum_record_count + 1) * sizeof(size_type);    // offsets

	LockedMemoryReference locked_reference;
	if(target_node == TARGET_NODE_UNSPECIFIED){
		locked_reference = memory_manager.allocate(buffer_size).lock();
	}else{
		locked_reference =
			memory_manager.allocate(buffer_size, target_node).lock();
	}
	const auto ptr =
		reinterpret_cast<uintptr_t>(locked_reference.pointer());

	const auto common_header =
		reinterpret_cast<SerializedBufferHeader *>(
			ptr + common_header_ptrdiff);
	common_header->key_buffer_size = 0;
	common_header->value_buffer_size =
		static_cast<size_type>(buffer_size - values_header_ptrdiff);

	const auto values_header =
		reinterpret_cast<SerializedValuesHeader *>(
			ptr + values_header_ptrdiff);
	values_header->data_buffer_size =
		static_cast<size_type>(
			values_offsets_ptrdiff - values_data_ptrdiff);
	values_header->maximum_record_count = maximum_record_count;
	values_header->actual_record_count = 0;

	return SerializedBuffer(locked_reference);
}

SerializedBuffer SerializedBuffer::allocate_key_value_buffer(
	MemoryManager &memory_manager,
	size_type maximum_record_count,
	size_type total_record_size,
	identifier_type target_node)
{
	size_type buffer_size = 0;
	// Common header
	const ptrdiff_t common_header_ptrdiff = buffer_size;
	buffer_size += sizeof(SerializedBufferHeader);
	// Values
	const ptrdiff_t values_header_ptrdiff = buffer_size;
	buffer_size += sizeof(SerializedValuesHeader);
	buffer_size  = align_ceil(buffer_size, alignof(max_align_t));
	const ptrdiff_t values_data_ptrdiff = buffer_size;
	buffer_size += align_ceil(total_record_size, alignof(size_type)); // data
	const ptrdiff_t values_offsets_ptrdiff = buffer_size;
	buffer_size += (maximum_record_count + 1) * sizeof(size_type);    // offsets
	buffer_size += maximum_record_count * sizeof(size_type);          // key_lengths

	LockedMemoryReference locked_reference;
	if(target_node == TARGET_NODE_UNSPECIFIED){
		locked_reference = memory_manager.allocate(buffer_size).lock();
	}else{
		locked_reference =
			memory_manager.allocate(buffer_size, target_node).lock();
	}
	const auto ptr =
		reinterpret_cast<uintptr_t>(locked_reference.pointer());

	const auto common_header =
		reinterpret_cast<SerializedBufferHeader *>(
			ptr + common_header_ptrdiff);
	common_header->key_buffer_size = 0;
	common_header->value_buffer_size =
		static_cast<size_type>(buffer_size - values_header_ptrdiff);
	
	const auto values_header =
		reinterpret_cast<SerializedValuesHeader *>(
			ptr + values_header_ptrdiff);
	values_header->data_buffer_size =
		static_cast<size_type>(
			values_offsets_ptrdiff - values_data_ptrdiff);
	values_header->maximum_record_count = maximum_record_count;
	values_header->actual_record_count = 0;

	return SerializedBuffer(locked_reference);
}

SerializedBuffer SerializedBuffer::allocate_grouped_buffer(
	MemoryManager &memory_manager,
	size_type maximum_record_count,
	size_type maximum_group_count,
	size_type total_key_size,
	size_type total_value_size,
	identifier_type target_node)
{
	size_type buffer_size = 0;
	// Common header
	const ptrdiff_t common_header_ptrdiff = buffer_size;
	buffer_size += sizeof(SerializedBufferHeader);
	// Keys
	const ptrdiff_t keys_header_ptrdiff = buffer_size;
	buffer_size += sizeof(SerializedKeysHeader);
	buffer_size  = align_ceil(buffer_size, alignof(max_align_t));
	const ptrdiff_t keys_data_ptrdiff = buffer_size;
	buffer_size += align_ceil(total_key_size, alignof(size_type)); // data
	const ptrdiff_t keys_offsets_ptrdiff = buffer_size;
	buffer_size += (maximum_group_count + 1) * sizeof(size_type);  // offsets
	// Values
	const ptrdiff_t values_header_ptrdiff = buffer_size;
	buffer_size += sizeof(SerializedValuesHeader);
	buffer_size  = align_ceil(buffer_size, alignof(max_align_t));
	const ptrdiff_t values_data_ptrdiff = buffer_size;
	buffer_size += align_ceil(total_value_size, alignof(size_type)); // data
	const ptrdiff_t values_offsets_ptrdiff = buffer_size;
	buffer_size += (maximum_record_count + 1) * sizeof(size_type);   // offsets
	buffer_size += (maximum_group_count + 1) * sizeof(size_type);   // group_offsets

	LockedMemoryReference locked_reference;
	if(target_node == TARGET_NODE_UNSPECIFIED){
		locked_reference = memory_manager.allocate(buffer_size).lock();
	}else{
		locked_reference =
			memory_manager.allocate(buffer_size, target_node).lock();
	}
	const auto ptr =
		reinterpret_cast<uintptr_t>(locked_reference.pointer());

	const auto common_header =
		reinterpret_cast<SerializedBufferHeader *>(
			ptr + common_header_ptrdiff);
	common_header->key_buffer_size =
		static_cast<size_type>(
			values_header_ptrdiff - keys_header_ptrdiff);
	common_header->value_buffer_size =
		static_cast<size_type>(buffer_size - values_header_ptrdiff);

	const auto keys_header =
		reinterpret_cast<SerializedKeysHeader *>(
			ptr + keys_header_ptrdiff);
	keys_header->data_buffer_size =
		static_cast<size_type>(keys_offsets_ptrdiff - keys_data_ptrdiff);
	keys_header->record_count = maximum_group_count;

	const auto values_header =
		reinterpret_cast<SerializedValuesHeader *>(
			ptr + values_header_ptrdiff);
	values_header->data_buffer_size =
		static_cast<size_type>(
			values_offsets_ptrdiff - values_data_ptrdiff);
	values_header->maximum_record_count = maximum_record_count;
	values_header->actual_record_count = 0;

	return SerializedBuffer(locked_reference);
}


SerializedBuffer::SerializedBuffer()
	: m_common_header(nullptr)
	, m_keys_header(nullptr)
	, m_values_header(nullptr)
	, m_keys_data(nullptr)
	, m_keys_offsets(nullptr)
	, m_values_data(nullptr)
	, m_values_offsets(nullptr)
	, m_values_key_lengths(nullptr)
	, m_values_group_offsets(nullptr)
{ }

SerializedBuffer::SerializedBuffer(LockedMemoryReference mobj)
	: m_memory_object(std::move(mobj))
	, m_common_header(nullptr)
	, m_keys_header(nullptr)
	, m_values_header(nullptr)
	, m_keys_data(nullptr)
	, m_keys_offsets(nullptr)
	, m_values_data(nullptr)
	, m_values_offsets(nullptr)
	, m_values_key_lengths(nullptr)
	, m_values_group_offsets(nullptr)
{
	if(!m_memory_object){ return; }
	const auto base_ptr =
		reinterpret_cast<uintptr_t>(m_memory_object.pointer());
	ptrdiff_t cur_offset = 0;

	m_common_header =
		reinterpret_cast<SerializedBufferHeader *>(base_ptr + cur_offset);
	cur_offset += sizeof(SerializedBufferHeader);
	const auto tail_offset = static_cast<ptrdiff_t>(
		sizeof(SerializedBufferHeader) +
		m_common_header->key_buffer_size +
		m_common_header->value_buffer_size);

	if(m_common_header->key_buffer_size > 0){
		m_keys_header =
			reinterpret_cast<SerializedKeysHeader *>(base_ptr + cur_offset);
		cur_offset += sizeof(SerializedKeysHeader);
		cur_offset  = align_ceil(cur_offset, alignof(max_align_t));
		m_keys_data = reinterpret_cast<void *>(base_ptr + cur_offset);
		cur_offset += m_keys_header->data_buffer_size;
		m_keys_offsets = reinterpret_cast<size_type *>(base_ptr + cur_offset);
		cur_offset +=
			sizeof(size_type) * (m_keys_header->record_count + 1);
	}

	if(m_common_header->value_buffer_size > 0){
		m_values_header =
			reinterpret_cast<SerializedValuesHeader *>(base_ptr + cur_offset);
		cur_offset += sizeof(SerializedValuesHeader);
		cur_offset  = align_ceil(cur_offset, alignof(max_align_t));
		m_values_data = reinterpret_cast<void *>(base_ptr + cur_offset);
		cur_offset += m_values_header->data_buffer_size;
		m_values_offsets =
			reinterpret_cast<size_type *>(base_ptr + cur_offset);
		cur_offset +=
			sizeof(size_type) * (m_values_header->maximum_record_count + 1);
		if(cur_offset == tail_offset){
			// value-only
		}else if(m_common_header->key_buffer_size == 0){
			// key-value
			m_values_key_lengths =
				reinterpret_cast<size_type *>(base_ptr + cur_offset);
			cur_offset +=
				sizeof(size_type) * m_values_header->maximum_record_count;
		}else{
			// grouped
			m_values_group_offsets =
				reinterpret_cast<size_type *>(base_ptr + cur_offset);
			cur_offset +=
				sizeof(size_type) * (m_keys_header->record_count + 1);
		}
	}
}

namespace {

uint64_t compute_partial_hash(const uint8_t *ptr, size_type len){
	const int HASH_BASE = 1000000009;
	uint64_t hash = 0;
	for(identifier_type i = 0; i < len; ++i){
		hash = (hash * HASH_BASE) + ptr[i];
	}
	return hash;
}

}

uint64_t SerializedBuffer::compute_hash() const noexcept {
	static const int HASH_BASE = 1000000007;
	uint64_t hash = 0;
	if(is_grouped()){
		const auto kd = reinterpret_cast<const uint8_t *>(keys_data());
		const auto ko = keys_offsets();
		const auto vd = reinterpret_cast<const uint8_t *>(values_data());
		const auto vo = values_offsets();
		const auto vgo = value_group_offsets();
		for(identifier_type gi = 0, vi = 0; gi < group_count(); ++gi){
			uint64_t ghash = compute_partial_hash(kd + ko[gi], ko[gi + 1] - ko[gi]);
			while(vo[vi] < vgo[gi + 1]){
				ghash ^= compute_partial_hash(vd + vo[vi], vo[vi + 1] - vo[vi]);
				++vi;
			}
			hash = (hash * HASH_BASE) + ghash;
		}
	}else if(m_values_key_lengths){
		const auto vd = reinterpret_cast<const uint8_t *>(values_data());
		const auto vo = values_offsets();
		for(identifier_type i = 0; i < record_count(); ++i){
			hash ^=
				compute_partial_hash(vd + vo[i], vo[i + 1] - vo[i]) * HASH_BASE +
				m_values_key_lengths[i];
		}
	}else{
		const auto vd = reinterpret_cast<const uint8_t *>(values_data());
		const auto vo = values_offsets();
		for(identifier_type i = 0; i < record_count(); ++i){
			hash ^= compute_partial_hash(vd + vo[i], vo[i + 1] - vo[i]);
		}
	}
	return hash;
}

}

