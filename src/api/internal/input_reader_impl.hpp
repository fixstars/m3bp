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
#ifndef M3BP_API_INTERNAL_INPUT_READER_IMPL_HPP
#define M3BP_API_INTERNAL_INPUT_READER_IMPL_HPP

#include "m3bp/input_reader.hpp"
#include "api/internal/input_buffer_impl.hpp"
#include "memory/memory_reference.hpp"

namespace m3bp {
namespace internal {

class InputReaderImpl {

private:
	LockedMemoryReference m_reference;

public:
	InputReaderImpl()
		: m_reference()
	{ }


	InputBuffer raw_buffer(){
		return InputBufferImpl::wrap_impl(
			InputBufferImpl().bind(m_reference));
	}


	InputReaderImpl &set_fragment(LockedMemoryReference reference){
		m_reference = std::move(reference);
		return *this;
	}


	static InputReaderImpl &get_impl(InputReader &reader){
		return *reader.m_impl;
	}

	static InputReader wrap_impl(InputReaderImpl impl){
		return InputReader(std::move(impl));
	}

};

}
}

#endif

