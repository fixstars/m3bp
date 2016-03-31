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
#ifndef M3BP_MEMORY_SERIALIZED_BUFFER_HPP
#define M3BP_MEMORY_SERIALIZED_BUFFER_HPP

#include <cassert>
#include "m3bp/types.hpp"
#include "common/array_ref.hpp"
#include "memory/memory_reference.hpp"

namespace m3bp {

class MemoryManager;
class MemoryReference;
class LockedMemoryReference;

class SerializedBuffer {

private:
	struct SerializedBufferHeader {
		size_type key_buffer_size;
		size_type value_buffer_size;
	};

	struct SerializedKeysHeader {
		size_type data_buffer_size;
		size_type record_count;
	};

	struct SerializedValuesHeader {
		size_type data_buffer_size;
		size_type maximum_record_count;
		size_type actual_record_count;
	};

	LockedMemoryReference m_memory_object;

	SerializedBufferHeader *m_common_header;
	SerializedKeysHeader *m_keys_header;
	SerializedValuesHeader *m_values_header;

	void *m_keys_data;
	size_type *m_keys_offsets;

	void *m_values_data;
	size_type *m_values_offsets;
	size_type *m_values_key_lengths;
	size_type *m_values_group_offsets;

public:
	static const identifier_type TARGET_NODE_UNSPECIFIED =
		std::numeric_limits<identifier_type>::max();

	static SerializedBuffer allocate_value_only_buffer(
		MemoryManager &memory_manager,
		size_type maximum_record_count,
		size_type total_record_size,
		identifier_type target_node = TARGET_NODE_UNSPECIFIED);

	static SerializedBuffer allocate_key_value_buffer(
		MemoryManager &memory_manager,
		size_type maximum_record_count,
		size_type total_record_size,
		identifier_type target_node = TARGET_NODE_UNSPECIFIED);

	static SerializedBuffer allocate_grouped_buffer(
		MemoryManager &memory_manager,
		size_type maximum_record_count,
		size_type maximum_group_count,
		size_type total_key_size,
		size_type total_value_size,
		identifier_type target_node = TARGET_NODE_UNSPECIFIED);


	SerializedBuffer();
	explicit SerializedBuffer(LockedMemoryReference mobj);


	LockedMemoryReference raw_reference(){
		return m_memory_object;
	}


	explicit operator bool() const noexcept {
		return m_common_header != nullptr;
	}


	bool is_grouped() const noexcept {
		return m_keys_header != nullptr;
	}


	uint64_t compute_hash() const noexcept;


	size_type record_count() const noexcept {
		assert(m_values_header);
		return m_values_header->actual_record_count;
	}
	SerializedBuffer &record_count(size_type count) noexcept {
		assert(m_values_header);
		m_values_header->actual_record_count = count;
		return *this;
	}

	size_type maximum_record_count() const noexcept {
		assert(m_values_header);
		return m_values_header->maximum_record_count;
	}

	size_type group_count() const noexcept {
		assert(m_keys_header);
		return m_keys_header->record_count;
	}


	const void *keys_data() const noexcept {
		assert(m_keys_data);
		return m_keys_data;
	}
	void *keys_data() noexcept {
		assert(m_keys_data);
		return m_keys_data;
	}

	size_type keys_data_size() const noexcept {
		assert(m_keys_header);
		return m_keys_header->data_buffer_size;
	}

	ArrayRef<const size_type> keys_offsets() const noexcept {
		assert(m_keys_offsets && m_keys_header);
		const auto base = m_keys_offsets;
		const auto len = m_keys_header->record_count + 1;
		return ArrayRef<const size_type>(base, base + len);
	}
	ArrayRef<size_type> keys_offsets() noexcept {
		assert(m_keys_offsets && m_keys_header);
		const auto base = m_keys_offsets;
		const auto len = m_keys_header->record_count + 1;
		return ArrayRef<size_type>(base, base + len);
	}

	const void *values_data() const noexcept {
		assert(m_values_data);
		return m_values_data;
	}
	void *values_data() noexcept {
		assert(m_values_data);
		return m_values_data;
	}

	size_type values_data_size() const noexcept {
		assert(m_values_header);
		return m_values_header->data_buffer_size;
	}

	ArrayRef<const size_type> values_offsets() const noexcept {
		assert(m_values_offsets && m_values_header);
		const auto base = m_values_offsets;
		const auto len = m_values_header->maximum_record_count + 1;
		return ArrayRef<const size_type>(base, base + len);
	}
	ArrayRef<size_type> values_offsets() noexcept {
		assert(m_values_offsets && m_values_header);
		const auto base = m_values_offsets;
		const auto len = m_values_header->maximum_record_count + 1;
		return ArrayRef<size_type>(base, base + len);
	}

	ArrayRef<const size_type> key_lengths() const noexcept {
		assert(m_values_header);
		const auto base = m_values_key_lengths;
		const auto len = m_values_header->maximum_record_count;
		return ArrayRef<const size_type>(base, base + len);
	}
	ArrayRef<size_type> key_lengths() noexcept {
		assert(m_values_header);
		const auto base = m_values_key_lengths;
		const auto len = m_values_header->maximum_record_count;
		return ArrayRef<size_type>(base, base + len);
	}

	ArrayRef<const size_type> value_group_offsets() const noexcept {
		assert(m_keys_header && m_values_group_offsets);
		const auto base = m_values_group_offsets;
		const auto len = m_keys_header->record_count + 1;
		return ArrayRef<const size_type>(base, base + len);
	}
	ArrayRef<size_type> value_group_offsets() noexcept {
		assert(m_keys_header && m_values_group_offsets);
		const auto base = m_values_group_offsets;
		const auto len = m_keys_header->record_count + 1;
		return ArrayRef<size_type>(base, base + len);
	}

};

}

#endif

