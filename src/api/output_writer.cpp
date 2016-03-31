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
#include "m3bp/output_writer.hpp"
#include "api/internal/output_writer_impl.hpp"

namespace m3bp {

OutputWriter::OutputWriter()
	: m_impl(new internal::OutputWriterImpl())
{ }

OutputWriter::OutputWriter(internal::OutputWriterImpl &&impl)
	: m_impl(new internal::OutputWriterImpl(std::move(impl)))
{ }

OutputWriter::OutputWriter(OutputWriter &&) = default;

OutputWriter::~OutputWriter() = default;


OutputWriter &OutputWriter::operator=(OutputWriter &&) = default;


OutputBuffer OutputWriter::allocate_buffer(){
	return m_impl->allocate_buffer();
}

OutputBuffer OutputWriter::allocate_buffer(
	size_type min_data_size, size_type min_record_count)
{
	return m_impl->allocate_buffer(min_data_size, min_record_count);
}


void OutputWriter::flush_buffer(OutputBuffer &&buffer, size_type record_count){
	m_impl->flush_buffer(std::move(buffer), record_count);
}

}

