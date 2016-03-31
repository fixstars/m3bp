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
#ifndef M3BP_API_INTERNAL_INPUT_BUFFER_IMPL_HPP
#define M3BP_API_INTERNAL_INPUT_BUFFER_IMPL_HPP

#include <vector>
#include "m3bp/input_buffer.hpp"
#include "memory/memory_reference.hpp"
#include "memory/serialized_buffer.hpp"

namespace m3bp {
namespace internal {

class InputBufferImpl {

private:
	SerializedBuffer m_serialized;

public:
	InputBufferImpl()
		: m_serialized()
	{ }


	size_type key_buffer_size() const {
		if(!m_serialized){ return 0; }
		if(m_serialized.is_grouped()){
			const auto offsets = m_serialized.keys_offsets();
			return offsets[m_serialized.group_count()];
		}else{
			const auto offsets = m_serialized.values_offsets();
			return offsets[m_serialized.record_count()];
		}
	}

	size_type value_buffer_size() const {
		if(!m_serialized || !m_serialized.is_grouped()){ return 0; }
		const auto offsets = m_serialized.values_offsets();
		return offsets[m_serialized.record_count()];
	}


	size_type record_count() const {
		if(!m_serialized){ return 0; }
		if(m_serialized.is_grouped()){
			return m_serialized.group_count();
		}else{
			return m_serialized.record_count();
		}
	}


	const void *key_buffer() const {
		if(!m_serialized){ return nullptr; }
		if(m_serialized.is_grouped()){
			return m_serialized.keys_data();
		}else{
			return m_serialized.values_data();
		}
	}
	const size_type *key_offset_table() const {
		static const size_type empty_offset_table[1] = { 0 };
		if(!m_serialized){ return empty_offset_table; }
		if(m_serialized.is_grouped()){
			return m_serialized.keys_offsets().data();
		}else{
			return m_serialized.values_offsets().data();
		}
	}

	const void *value_buffer() const {
		if(!m_serialized || !m_serialized.is_grouped()){
			return nullptr;
		}
		return m_serialized.values_data();
	}
	const size_type *value_offset_table() const {
		static const size_type empty_offset_table[1] = { 0 };
		if(!m_serialized || !m_serialized.is_grouped()){
			return empty_offset_table;
		}
		return m_serialized.value_group_offsets().data();
	}


	InputBufferImpl &bind(LockedMemoryReference mobj){
		m_serialized = SerializedBuffer(std::move(mobj));
		return *this;
	}


	static InputBufferImpl &get_impl(InputBuffer &buffer){
		return *buffer.m_impl;
	}

	static InputBuffer wrap_impl(InputBufferImpl impl){
		return InputBuffer(std::move(impl));
	}

};

}
}

#endif

