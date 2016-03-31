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
#ifndef M3BP_API_INTERNAL_OUTPUT_BUFFER_IMPL_HPP
#define M3BP_API_INTERNAL_OUTPUT_BUFFER_IMPL_HPP

#include <memory>
#include <boost/noncopyable.hpp>
#include "m3bp/output_buffer.hpp"
#include "memory/memory_reference.hpp"
#include "memory/serialized_buffer.hpp"

namespace m3bp {
namespace internal {

class OutputBufferImpl {

private:
	SerializedBuffer m_serialized;

public:
	OutputBufferImpl()
		: m_serialized()
	{ }

	OutputBufferImpl(const OutputBufferImpl &) = delete;

	OutputBufferImpl(OutputBufferImpl &&) noexcept = default;


	OutputBufferImpl &operator=(const OutputBufferImpl &) = delete;

	OutputBufferImpl &operator=(OutputBufferImpl &&) noexcept = default;


	size_type data_buffer_size() const {
		if(!m_serialized){ return 0; }
		return m_serialized.values_data_size();
	}

	size_type max_record_count() const {
		if(!m_serialized){ return 0; }
		return m_serialized.maximum_record_count();
	}


	const void *data_buffer() const {
		if(!m_serialized){ return nullptr; }
		return m_serialized.values_data();
	}
	void *data_buffer(){
		if(!m_serialized){ return nullptr; }
		return m_serialized.values_data();
	}


	const size_type *offset_table() const {
		if(!m_serialized){ return nullptr; }
		return m_serialized.values_offsets().data();
	}
	size_type *offset_table(){
		if(!m_serialized){ return nullptr; }
		return m_serialized.values_offsets().data();
	}


	const size_type *key_length_table() const {
		if(!m_serialized){ return nullptr; }
		return m_serialized.key_lengths().data();
	}
	size_type *key_length_table(){
		if(!m_serialized){ return nullptr; }
		return m_serialized.key_lengths().data();
	}


	OutputBufferImpl &bind_fragment(SerializedBuffer fragment){
		m_serialized = SerializedBuffer(std::move(fragment));
		return *this;
	}

	SerializedBuffer unbind_fragment(){
		auto moved = std::move(m_serialized);
		m_serialized = SerializedBuffer();
		return moved;
	}


	static OutputBufferImpl &get_impl(OutputBuffer &buffer){
		return *buffer.m_impl;
	}

	static OutputBuffer wrap_impl(OutputBufferImpl &&impl){
		return OutputBuffer(std::move(impl));
	}

};

}
}

#endif

