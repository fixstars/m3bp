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
#include "m3bp/input_reader.hpp"
#include "api/internal/input_reader_impl.hpp"

namespace m3bp {

InputReader::InputReader()
	: m_impl(new internal::InputReaderImpl())
{ }

InputReader::InputReader(internal::InputReaderImpl &&impl)
	: m_impl(new internal::InputReaderImpl(std::move(impl)))
{ }

InputReader::InputReader(InputReader &&) = default;

InputReader::~InputReader() = default;


InputReader &InputReader::operator=(InputReader &&) = default;


InputBuffer InputReader::raw_buffer(){
	return m_impl->raw_buffer();
}

}

