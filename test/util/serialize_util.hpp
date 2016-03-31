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
#ifndef M3BP_TEST_SERIALIZE_UTIL_HPP
#define M3BP_TEST_SERIALIZE_UTIL_HPP

#include <gtest/gtest.h>
#include "memory/memory_manager.hpp"
#include "memory/serialized_buffer.hpp"
#include "binary_util.hpp"

namespace util {

//---------------------------------------------------------------------------
// Serialize/Deserialize a value-only buffer
//---------------------------------------------------------------------------
template <typename T>
static m3bp::LockedMemoryReference create_serialized_buffer(
	m3bp::MemoryManager &memory_manager,
	const std::vector<T> &data)
{
	// accumulate size of all records
	m3bp::size_type total_data_size = 0;
	for(const auto &x : data){ total_data_size += binary_length(x); }
	// allocate a buffer
	auto dst_sb = m3bp::SerializedBuffer::allocate_value_only_buffer(
		memory_manager, data.size(), total_data_size);
	// write serialized records to the buffer
	auto data_buffer = static_cast<uint8_t *>(dst_sb.values_data());
	auto offsets = dst_sb.values_offsets();
	offsets[0] = 0;
	for(m3bp::size_type i = 0; i < data.size(); ++i){
		const auto len = binary_length(data[i]);
		write_binary(data_buffer + offsets[i], data[i]);
		offsets[i + 1] = offsets[i] + len;
	}
	dst_sb.record_count(data.size());
	return dst_sb.raw_reference();
}

template <typename T>
static std::vector<T>
deserialize_value_only_buffer(m3bp::LockedMemoryReference mobj){
	// precondition: buffer is locked
	EXPECT_NE(nullptr, mobj.pointer());
	m3bp::SerializedBuffer sb(std::move(mobj));
	// Deserialize data
	std::vector<T> result;
	const auto data_buffer = static_cast<const uint8_t *>(sb.values_data());
	const auto offsets = sb.values_offsets();
	const auto record_count = sb.record_count();
	for(m3bp::size_type i = 0; i < record_count; ++i){
		const auto p = read_binary<T>(data_buffer + offsets[i]);
		result.emplace_back(p.first);
	}
	return result;
}

//---------------------------------------------------------------------------
// Serialize/Deserialize a key-value buffer
//---------------------------------------------------------------------------
template <typename KeyType, typename ValueType>
static m3bp::LockedMemoryReference create_serialized_buffer(
	m3bp::MemoryManager &memory_manager,
	const std::vector<std::pair<KeyType, ValueType>> &data)
{
	// accumulate size of all records
	m3bp::size_type total_data_size = 0;
	for(const auto &x : data){ total_data_size += binary_length(x); }
	// allocate a buffer
	auto dst_sb = m3bp::SerializedBuffer::allocate_key_value_buffer(
		memory_manager, data.size(), total_data_size);
	// write serialized records to the buffer
	auto data_buffer = static_cast<uint8_t *>(dst_sb.values_data());
	auto offsets = dst_sb.values_offsets();
	auto key_lengths = dst_sb.key_lengths();
	offsets[0] = 0;
	for(m3bp::size_type i = 0; i < data.size(); ++i){
		const auto len = binary_length(data[i]);
		write_binary(data_buffer + offsets[i], data[i]);
		offsets[i + 1] = offsets[i] + len;
		key_lengths[i] = binary_length(data[i].first);
	}
	dst_sb.record_count(data.size());
	return dst_sb.raw_reference();
}

template <typename KeyType, typename ValueType>
static std::vector<std::pair<KeyType, ValueType>>
deserialize_key_value_buffer(m3bp::LockedMemoryReference mobj){
	using PairType = std::pair<KeyType, ValueType>;
	// precondition: buffer is locked
	EXPECT_NE(nullptr, mobj.pointer());
	m3bp::SerializedBuffer sb(std::move(mobj));
	// Deserialize data
	std::vector<PairType> result;
	const auto data_buffer = static_cast<const uint8_t *>(sb.values_data());
	const auto offsets = sb.values_offsets();
	const auto key_lengths = sb.key_lengths();
	const auto record_count = sb.record_count();
	for(m3bp::size_type i = 0; i < record_count; ++i){
		const auto p = read_binary<PairType>(data_buffer + offsets[i]);
		EXPECT_EQ(util::binary_length(p.first), key_lengths[i]);
		result.emplace_back(p.first);
	}
	return result;
}

//---------------------------------------------------------------------------
// Serialize/Deserialize a grouped buffer
//---------------------------------------------------------------------------
template <typename KeyType, typename ValueType>
static m3bp::LockedMemoryReference create_serialized_buffer(
	m3bp::MemoryManager &memory_manager,
	const std::vector<KeyType> &keys,
	const std::vector<std::vector<ValueType>> &values)
{
	assert(keys.size() == values.size());
	// accumulate size of all records
	m3bp::size_type total_keys_size = 0;
	m3bp::size_type total_values_size = 0;
	m3bp::size_type total_values_count = 0;
	for(const auto &x : keys){ total_keys_size += binary_length(x); }
	for(const auto &v : values){
		total_values_count += v.size();
		for(const auto &x : v){ total_values_size += binary_length(x); }
	}
	// allocate a buffer
	auto dst_sb = m3bp::SerializedBuffer::allocate_grouped_buffer(
		memory_manager, total_values_count, keys.size(),
		total_keys_size, total_values_size);
	// write serialized keys to the buffer
	auto keys_buffer = static_cast<uint8_t *>(dst_sb.keys_data());
	auto keys_offsets = dst_sb.keys_offsets();
	keys_offsets[0] = 0;
	for(m3bp::size_type i = 0; i < keys.size(); ++i){
		const auto len = binary_length(keys[i]);
		write_binary(keys_buffer + keys_offsets[i], keys[i]);
		keys_offsets[i + 1] = keys_offsets[i] + len;
	}
	dst_sb.record_count(total_values_count);
	// write serialized values to the buffer
	auto values_buffer = static_cast<uint8_t *>(dst_sb.values_data());
	auto values_offsets = dst_sb.values_offsets();
	auto groups_offsets = dst_sb.value_group_offsets();
	values_offsets[0] = groups_offsets[0] = 0;
	for(m3bp::size_type i = 0, j = 0; i < values.size(); ++i){
		groups_offsets[i + 1] = groups_offsets[i];
		for(const auto &x : values[i]){
			const auto len = binary_length(x);
			write_binary(values_buffer + values_offsets[j], x);
			values_offsets[j + 1] = values_offsets[j] + len;
			groups_offsets[i + 1] += len;
			++j;
		}
	}
	return dst_sb.raw_reference();
}

template <typename KeyType, typename ValueType>
static std::vector<std::pair<KeyType, std::vector<ValueType>>>
deserialize_grouped_buffer(m3bp::LockedMemoryReference mobj){
	using PairType = std::pair<KeyType, std::vector<ValueType>>;
	// precondition: buffer is locked
	EXPECT_NE(nullptr, mobj.pointer());
	m3bp::SerializedBuffer sb(std::move(mobj));
	// Deserialize data
	std::vector<PairType> result;
	const auto keys_buffer = static_cast<const uint8_t *>(sb.keys_data());
	const auto keys_offsets = sb.keys_offsets();
	const auto values_buffer = static_cast<const uint8_t *>(sb.values_data());
	const auto values_offsets = sb.values_offsets();
	const auto group_offsets = sb.value_group_offsets();
	for(m3bp::size_type i = 0, j = 0; i + 1 < keys_offsets.size(); ++i){
		const KeyType key =
			read_binary<KeyType>(keys_buffer + keys_offsets[i]).first;
		std::vector<ValueType> values;
		while(values_offsets[j] < group_offsets[i + 1]){
			const auto p =
				read_binary<ValueType>(values_buffer + values_offsets[j]);
			values.emplace_back(p.first);
			++j;
		}
		result.emplace_back(std::move(key), std::move(values));
	}
	return result;
}

}

#endif

