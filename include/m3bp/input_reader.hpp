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
#ifndef M3BP_INPUT_READER_HPP
#define M3BP_INPUT_READER_HPP

#include <memory>
#include "m3bp/input_buffer.hpp"

namespace m3bp {

namespace internal {

class InputReaderImpl;

}

/**
 */
class InputReader {

public:
	/**
	 *  Constructs a reader that is not binded with any buffers.
	 */
	InputReader();

	InputReader(const InputReader &) = delete;

	/**
	 *  Move constructor.
	 */
	InputReader(InputReader &&reader);

	/**
	 *  Destructor.
	 */
	~InputReader();


	InputReader &operator=(const InputReader &) = delete;

	/**
	 *  Move substitution operator.
	 */
	InputReader &operator=(InputReader &&reader);


	/**
	 *  Gets an {@link InputBuffer} binded to this reader.
	 *
	 *  @return An {@link InputBuffer} binded to this reader.
	 */
	InputBuffer raw_buffer();

private:
	friend class internal::InputReaderImpl;
	explicit InputReader(internal::InputReaderImpl &&impl);
	std::unique_ptr<internal::InputReaderImpl> m_impl;

};

}

#endif

