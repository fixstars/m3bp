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
#ifndef M3BP_OUTPUT_WRITER_HPP
#define M3BP_OUTPUT_WRITER_HPP

#include <memory>
#include "m3bp/types.hpp"
#include "m3bp/output_buffer.hpp"

namespace m3bp {

namespace internal {

class OutputWriterImpl;

}

/**
 *  An interface for writing output data.
 */
class OutputWriter {

public:
	/**
	 *  Constructs an invalid writer.
	 */
	OutputWriter();

	OutputWriter(const OutputWriter &) = delete;

	/**
	 *  Move constructor
	 */
	OutputWriter(OutputWriter &&writer);

	/**
	 *  Destructs the writer.
	 */
	~OutputWriter();


	OutputWriter &operator=(const OutputWriter &) = delete;

	/**
	 *  Move substitution operator.
	 */
	OutputWriter &operator=(OutputWriter &&writer);


	/**
	 *  Allocates a buffer for writing output data.
	 *
	 *  @return An allocated buffer object.
	 */
	OutputBuffer allocate_buffer();

	/**
	 *  Allocates a buffer for writing output data.
	 *
	 *  @param[in] min_data_size     Minimum size of the data buffer in bytes.
	 *  @param[in] min_record_count  Minimum number of records such that the
	 *                               buffer can keep.
	 *  @return    An allocated buffer object.
	 */
	OutputBuffer allocate_buffer(
		size_type min_data_size, size_type min_record_count);


	/**
	 *  Flushes data written in the buffer.
	 *
	 *  @param[in] buffer        A buffer object to flush.
	 *  @param[in] record_count  The number of records that is stored on buffer.
	 */
	void flush_buffer(OutputBuffer &&buffer, size_type record_count);

private:
	friend class internal::OutputWriterImpl;
	explicit OutputWriter(internal::OutputWriterImpl &&impl);
	std::unique_ptr<internal::OutputWriterImpl> m_impl;

};

}

#endif

