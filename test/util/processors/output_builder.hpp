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
#ifndef M3BP_TEST_UTIL_PROCESSORS_OUTPUT_BUILDER_HPP
#define M3BP_TEST_UTIL_PROCESSORS_OUTPUT_BUILDER_HPP

#include <vector>
#include <cstdint>
#include "m3bp/output_writer.hpp"
#include "m3bp/output_buffer.hpp"
#include "util/binary_util.hpp"

namespace util { 
namespace processors {

template <typename T>
m3bp::OutputBuffer build_output_buffer(
	m3bp::OutputWriter &writer, const std::vector<T> &records)
{
	m3bp::size_type binary_size = 0;
	for(const auto &x : records){ binary_size += binary_length(x); }
	auto buffer = writer.allocate_buffer(binary_size, records.size());
	const auto ptr = static_cast<uint8_t *>(buffer.data_buffer());
	const auto offsets = buffer.offset_table();
	offsets[0] = 0;
	for(m3bp::identifier_type i = 0; i < records.size(); ++i){
		write_binary(ptr + offsets[i], records[i]);
		offsets[i + 1] = offsets[i] + binary_length(records[i]);
	}
	return buffer;
}

template <typename KeyType, typename ValueType>
m3bp::OutputBuffer build_output_buffer(
	m3bp::OutputWriter &writer,
	const std::vector<std::pair<KeyType, ValueType>> &records)
{
	m3bp::size_type binary_size = 0;
	for(const auto &x : records){ binary_size += binary_length(x); }
	auto buffer = writer.allocate_buffer(binary_size, records.size());
	const auto ptr = static_cast<uint8_t *>(buffer.data_buffer());
	const auto offsets = buffer.offset_table();
	const auto key_lengths = buffer.key_length_table();
	offsets[0] = 0;
	for(m3bp::identifier_type i = 0; i < records.size(); ++i){
		write_binary(ptr + offsets[i], records[i]);
		offsets[i + 1] = offsets[i] + binary_length(records[i]);
		key_lengths[i] = binary_length(records[i].first);
	}
	return buffer;
}

}
}

#endif

