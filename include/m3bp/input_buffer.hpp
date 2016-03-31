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
#ifndef M3BP_INPUT_BUFFER_HPP
#define M3BP_INPUT_BUFFER_HPP

#include <memory>
#include "m3bp/types.hpp"

namespace m3bp {

namespace internal {

class InputBufferImpl;

}

/**
 *  An buffer for processors to receive input data.
 */
class InputBuffer {

public:
	/**
	 *  Constructs an empty buffer.
	 */
	InputBuffer();

	InputBuffer(const InputBuffer &) = delete;

	/**
	 *  Move constructor.
	 */
	InputBuffer(InputBuffer &&buffer);

	/**
	 *  Destructor.
	 */
	~InputBuffer();


	InputBuffer &operator=(const InputBuffer &) = delete;

	/**
	 *  Move substitution operator.
	 */
	InputBuffer &operator=(InputBuffer &&buffer);


	/**
	 *  Gets the size of the key buffer in bytes.
	 *
	 *  @return The size of the key buffer in bytes.
	 */
	size_type key_buffer_size() const;

	/**
	 *  Gets the size of the value buffer in bytes.
	 *
	 *  @return The size of the value buffer in bytes.
	 */
	size_type value_buffer_size() const;


	/**
	 *  Gets the number of records or groups in this buffer.
	 *
	 *  If this buffer is corresponding to a scatter-gather port, this method
	 *  returns the number of groups in this buffer. Otherwise, this method
	 *  returns the number of records in this buffer.
	 *
	 *  @return The number of records or groups in this buffer.
	 */
	size_type record_count() const;


	/**
	 *  Gets the buffer that contains keys.
	 */
	const void *key_buffer() const;
	const size_type *key_offset_table() const;

	const void *value_buffer() const;
	const size_type *value_offset_table() const;

private:
	friend class internal::InputBufferImpl;
	explicit InputBuffer(internal::InputBufferImpl &&impl);
	std::unique_ptr<internal::InputBufferImpl> m_impl;

};

}

#endif

