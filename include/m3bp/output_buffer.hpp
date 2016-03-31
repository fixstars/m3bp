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
#ifndef M3BP_OUTPUT_BUFFER_HPP
#define M3BP_OUTPUT_BUFFER_HPP

#include <memory>
#include "m3bp/types.hpp"

namespace m3bp {

namespace internal {

class OutputBufferImpl;

}

/**
 *  A buffer for passing records to the engine.
 */
class OutputBuffer {

public:
	/**
	 *  Constructs an empty buffer.
	 */
	OutputBuffer();

	OutputBuffer(const OutputBuffer &) = delete;

	/**
	 *  Move constructor.
	 */
	OutputBuffer(OutputBuffer &&buffer);

	/**
	 *  Destructor.
	 */
	~OutputBuffer();


	OutputBuffer &operator=(const OutputBuffer &) = delete;

	/**
	 *  Move substitution operator.
	 */
	OutputBuffer &operator=(OutputBuffer &&buffer);


	/**
	 *  Gets the size of data buffer in bytes.
	 *
	 *  @return The size of data buffer in bytes.
	 */
	size_type data_buffer_size() const;

	/**
	 *  Gets the maximum number of records in this buffer.
	 *
	 *  @return The maximum number of records in this buffer.
	 */
	size_type max_record_count() const;


	/**
	 *  Gets the pointer to data buffer.
	 *
	 *  @return The pointer to data buffer.
	 */
	const void *data_buffer() const;

	/**
	 *  Gets the pointer to data buffer.
	 *
	 *  @return The pointer to data buffer.
	 */
	void *data_buffer();


	/**
	 *  Gets the pointer to data offset table.
	 *
	 *  i-th element of this table is the offset of the head of i-th record
	 *  and/or tail of (i-1)-th record.
	 *
	 *  @return The pointer to data offset table.
	 */
	const size_type *offset_table() const;

	/**
	 *  Gets the pointer to data offset table.
	 *
	 *  i-th element of this table is the offset of the head of i-th record
	 *  and/or tail of (i-1)-th record.
	 *
	 *  @return The pointer to data offset table.
	 */
	size_type *offset_table();


	/**
	 *  Gets the pointer to the key length table.
	 *
	 *  If records in this buffer has keys, first key_length_table[i] bytes of
	 *  the i-th record are treated as the key of i-th record.
	 *
	 *  If the output port corresponding to this buffer does not have keys,
	 *  this method will returns @c nullptr.
	 *
	 *  @retrun The pointer to the key length table.
	 */
	const size_type *key_length_table() const;

	/**
	 *  Gets the pointer to the key length table.
	 *
	 *  If records in this buffer has keys, first key_length_table[i] bytes of
	 *  the i-th record are treated as the key of i-th record.
	 *
	 *  If the output port corresponding to this buffer does not have keys,
	 *  this method will returns @c nullptr.
	 *
	 *  @retrun The pointer to the key length table.
	 */
	size_type *key_length_table();

private:
	friend class internal::OutputBufferImpl;
	OutputBuffer(internal::OutputBufferImpl &&impl);
	std::unique_ptr<internal::OutputBufferImpl> m_impl;

};

}

#endif

