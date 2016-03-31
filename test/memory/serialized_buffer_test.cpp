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
#include <vector>
#include <utility>
#include <gtest/gtest.h>
#include "memory/serialized_buffer.hpp"
#include "memory/memory_manager.hpp"
#include "memory/memory_reference.hpp"

namespace {

using PointerRange = std::pair<uintptr_t, uintptr_t>;

template <typename Iterator>
void test_ranges(Iterator begin, Iterator end){
	for(Iterator it = begin; it != end; ++it){
		EXPECT_LE(it->first, it->second);
		std::fill(
			reinterpret_cast<uint8_t *>(it->first),
			reinterpret_cast<uint8_t *>(it->second),
			0xff);
	}
	for(Iterator it = begin; it != end; ++it){
		for(Iterator jt = begin; jt != end; ++jt){
			if(it == jt){ continue; }
			EXPECT_TRUE(
				jt->second <= it->first ||
				jt->first >= it->second);
		}
	}
}

}

TEST(SerializedBuffer, AllocateValueOnly){
	auto memory_manager = std::make_shared<m3bp::MemoryManager>();
	auto &mm = *memory_manager;
	std::vector<m3bp::size_type> a = { 41, 53 };

	do {
		const auto maximum_record_count = a[0];
		const auto total_record_size = a[1];

		m3bp::MemoryReference unlocked_reference;
		{
			auto generated_sb =
				m3bp::SerializedBuffer::allocate_value_only_buffer(
					mm, maximum_record_count, total_record_size);
			EXPECT_NE(0u, mm.total_memory_usage());
			EXPECT_TRUE(generated_sb);
			unlocked_reference =
				m3bp::MemoryReference(generated_sb.raw_reference());
		}
		auto reference = unlocked_reference.lock();
		EXPECT_NE(nullptr, reference.pointer());

		m3bp::SerializedBuffer sb(reference);
		std::vector<PointerRange> ranges;

		const auto values_head = reinterpret_cast<uintptr_t>(sb.values_data());
		const auto values_len = sb.values_data_size();
		EXPECT_GE(values_len, total_record_size);
		ranges.emplace_back(values_head, values_head + values_len);

		const auto offsets = sb.values_offsets();
		EXPECT_GE(offsets.size(), maximum_record_count + 1);
		ranges.emplace_back(
			reinterpret_cast<uintptr_t>(offsets.begin()),
			reinterpret_cast<uintptr_t>(offsets.end()));

		test_ranges(ranges.begin(), ranges.end());
	} while(next_permutation(a.begin(), a.end()));

	EXPECT_EQ(0u, mm.total_memory_usage());
}

TEST(SerializedBuffer, AllocateKeyValue){
	auto memory_manager = std::make_shared<m3bp::MemoryManager>();
	auto &mm = *memory_manager;
	std::vector<m3bp::size_type> a = { 41, 53 };

	do {
		const auto maximum_record_count = a[0];
		const auto total_record_size = a[1];

		m3bp::MemoryReference unlocked_reference;
		{
			auto generated_sb =
				m3bp::SerializedBuffer::allocate_key_value_buffer(
					mm, maximum_record_count, total_record_size);
			EXPECT_NE(0u, mm.total_memory_usage());
			EXPECT_TRUE(generated_sb);
			unlocked_reference =
				m3bp::MemoryReference(generated_sb.raw_reference());
		}
		auto reference = unlocked_reference.lock();
		EXPECT_NE(nullptr, reference.pointer());

		m3bp::SerializedBuffer sb(reference);
		std::vector<PointerRange> ranges;

		const auto values_head = reinterpret_cast<uintptr_t>(sb.values_data());
		const auto values_len = sb.values_data_size();
		EXPECT_GE(values_len, total_record_size);
		ranges.emplace_back(values_head, values_head + values_len);

		const auto offsets = sb.values_offsets();
		EXPECT_GE(offsets.size(), maximum_record_count + 1);
		ranges.emplace_back(
			reinterpret_cast<uintptr_t>(offsets.begin()),
			reinterpret_cast<uintptr_t>(offsets.end()));

		const auto key_lengths = sb.key_lengths();
		EXPECT_GE(key_lengths.size(), maximum_record_count);
		ranges.emplace_back(
			reinterpret_cast<uintptr_t>(key_lengths.begin()),
			reinterpret_cast<uintptr_t>(key_lengths.end()));

		test_ranges(ranges.begin(), ranges.end());
	} while(next_permutation(a.begin(), a.end()));

	EXPECT_EQ(0u, mm.total_memory_usage());
}

TEST(SerializedBuffer, AllocateGrouped){
	auto memory_manager = std::make_shared<m3bp::MemoryManager>();
	auto &mm = *memory_manager;
	std::vector<m3bp::size_type> a = { 41, 53, 71, 83 };

	do {
		const auto maximum_record_count = a[0];
		const auto maximum_group_count = a[1];
		const auto total_key_size = a[2];
		const auto total_value_size = a[3];
		if(maximum_record_count < maximum_group_count){
			continue;
		}

		m3bp::MemoryReference unlocked_reference;
		{
			auto generated_sb =
				m3bp::SerializedBuffer::allocate_grouped_buffer(
					mm, maximum_record_count, maximum_group_count,
					total_key_size, total_value_size);
			EXPECT_NE(0u, mm.total_memory_usage());
			EXPECT_TRUE(generated_sb);
			unlocked_reference =
				m3bp::MemoryReference(generated_sb.raw_reference());
		}
		auto reference = unlocked_reference.lock();
		EXPECT_NE(nullptr, reference.pointer());

		m3bp::SerializedBuffer sb(reference);
		std::vector<PointerRange> ranges;

		const auto keys_head = reinterpret_cast<uintptr_t>(sb.keys_data());
		const auto keys_len = sb.keys_data_size();
		EXPECT_GE(keys_len, total_key_size);
		ranges.emplace_back(keys_head, keys_head + keys_len);

		const auto key_offsets = sb.keys_offsets();
		EXPECT_GE(key_offsets.size(), maximum_group_count + 1);
		ranges.emplace_back(
			reinterpret_cast<uintptr_t>(key_offsets.begin()),
			reinterpret_cast<uintptr_t>(key_offsets.end()));

		const auto values_head = reinterpret_cast<uintptr_t>(sb.values_data());
		const auto values_len = sb.values_data_size();
		EXPECT_GE(values_len, total_value_size);
		ranges.emplace_back(values_head, values_head + values_len);

		const auto value_offsets = sb.values_offsets();
		EXPECT_GE(value_offsets.size(), maximum_record_count + 1);
		ranges.emplace_back(
			reinterpret_cast<uintptr_t>(value_offsets.begin()),
			reinterpret_cast<uintptr_t>(value_offsets.end()));

		const auto group_offsets = sb.value_group_offsets();
		EXPECT_GE(group_offsets.size(), maximum_group_count + 1);
		ranges.emplace_back(
			reinterpret_cast<uintptr_t>(group_offsets.begin()),
			reinterpret_cast<uintptr_t>(group_offsets.end()));

		test_ranges(ranges.begin(), ranges.end());
	} while(next_permutation(a.begin(), a.end()));

	EXPECT_EQ(0u, mm.total_memory_usage());
}

