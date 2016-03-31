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

#include "m3bp/output_buffer.hpp"
#include "api/internal/output_buffer_impl.hpp"

namespace m3bp {

OutputBuffer::OutputBuffer()
	: m_impl(new internal::OutputBufferImpl())
{ }

OutputBuffer::OutputBuffer(internal::OutputBufferImpl &&impl)
	: m_impl(new internal::OutputBufferImpl(std::move(impl)))
{ }

OutputBuffer::OutputBuffer(OutputBuffer &&) = default;

OutputBuffer::~OutputBuffer() = default;


OutputBuffer &OutputBuffer::operator=(OutputBuffer &&) = default;


size_type OutputBuffer::data_buffer_size() const {
	if(!m_impl){ return 0; }
	return m_impl->data_buffer_size();
}

size_type OutputBuffer::max_record_count() const {
	if(!m_impl){ return 0; }
	return m_impl->max_record_count();
}


const void *OutputBuffer::data_buffer() const {
	if(!m_impl){ return nullptr; }
	return m_impl->data_buffer();
}

void *OutputBuffer::data_buffer(){
	if(!m_impl){ return nullptr; }
	return m_impl->data_buffer();
}


const size_type *OutputBuffer::offset_table() const {
	if(!m_impl){ return nullptr; }
	return m_impl->offset_table();
}

size_type *OutputBuffer::offset_table(){
	if(!m_impl){ return nullptr; }
	return m_impl->offset_table();
}


const size_type *OutputBuffer::key_length_table() const {
	if(!m_impl){ return nullptr; }
	return m_impl->key_length_table();
}

size_type *OutputBuffer::key_length_table(){
	if(!m_impl){ return nullptr; }
	return m_impl->key_length_table();
}

}

