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
#include "m3bp/input_buffer.hpp"
#include "api/internal/input_buffer_impl.hpp"

namespace m3bp {

InputBuffer::InputBuffer()
	: m_impl(new internal::InputBufferImpl())
{ }

InputBuffer::InputBuffer(internal::InputBufferImpl &&impl)
	: m_impl(new internal::InputBufferImpl(std::move(impl)))
{ }

InputBuffer::InputBuffer(InputBuffer &&) = default;

InputBuffer::~InputBuffer() = default;


InputBuffer &InputBuffer::operator=(InputBuffer &&) = default;


size_type InputBuffer::key_buffer_size() const {
	return m_impl->key_buffer_size();
}

size_type InputBuffer::value_buffer_size() const {
	return m_impl->value_buffer_size();
}


size_type InputBuffer::record_count() const {
	return m_impl->record_count();
}


const void *InputBuffer::key_buffer() const {
	return m_impl->key_buffer();
}

const size_type *InputBuffer::key_offset_table() const {
	return m_impl->key_offset_table();
}


const void *InputBuffer::value_buffer() const {
	return m_impl->value_buffer();
}

const size_type *InputBuffer::value_offset_table() const {
	return m_impl->value_offset_table();
}

}

