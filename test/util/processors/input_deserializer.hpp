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
#ifndef M3BP_TEST_UTIL_PROCESSORS_INPUT_DESERIALIZER_HPP
#define M3BP_TEST_UTIL_PROCESSORS_INPUT_DESERIALIZER_HPP

#include <vector>
#include <cstdint>
#include "m3bp/input_reader.hpp"
#include "m3bp/input_buffer.hpp"
#include "util/binary_util.hpp"

namespace util { 
namespace processors {

template <typename T>
std::vector<T> deserialize_value_only_buffer(m3bp::InputReader &reader){
	const auto buffer = reader.raw_buffer();
	const auto num_records = buffer.record_count();
	const auto ptr = static_cast<const uint8_t *>(buffer.key_buffer());
	const auto offsets = buffer.key_offset_table();
	std::vector<T> result(num_records);
	for(m3bp::identifier_type i = 0; i < num_records; ++i){
		result[i] = read_binary<T>(ptr + offsets[i]).first;
	}
	return result;
}

template <typename KeyType, typename ValueType>
std::vector<std::pair<KeyType, std::vector<ValueType>>>
deserialize_grouped_buffer(m3bp::InputReader &reader){
	using GroupType = std::pair<KeyType, std::vector<ValueType>>;
	const auto buffer = reader.raw_buffer();
	const auto num_groups = buffer.record_count();
	const auto key_ptr = static_cast<const uint8_t *>(buffer.key_buffer());
	const auto key_offsets = buffer.key_offset_table();
	const auto value_ptr = static_cast<const uint8_t *>(buffer.value_buffer());
	const auto value_offsets = buffer.value_offset_table();
	std::vector<GroupType> result;
	for(m3bp::identifier_type i = 0; i < num_groups; ++i){
		const auto key = read_binary<KeyType>(key_ptr + key_offsets[i]);
		std::vector<ValueType> values;
		const auto head = value_offsets[i], tail = value_offsets[i + 1];
		for(m3bp::size_type cur = head; cur < tail; ){
			const auto x = read_binary<ValueType>(value_ptr + cur);
			values.push_back(x.first);
			cur += x.second;
		}
		result.emplace_back(key.first, std::move(values));
	}
	return result;
}

}
}

#endif

