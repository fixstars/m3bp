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
#include <gtest/gtest.h>
#include "m3bp/output_buffer.hpp"
#include "api/internal/output_buffer_impl.hpp"
#include "memory/memory_manager.hpp"
#include "memory/memory_reference.hpp"
#include "memory/serialized_buffer.hpp"

TEST(OutputBuffer, Parameters){
	const m3bp::size_type record_count = 200;
	const m3bp::size_type data_buffer_size = 1000;
	auto memory_manager = std::make_shared<m3bp::MemoryManager>();
	{
		auto sb = m3bp::SerializedBuffer::allocate_key_value_buffer(
			*memory_manager, record_count, data_buffer_size);
		const auto data_buffer = sb.values_data();
		const auto offset_table = sb.values_offsets().data();
		const auto key_lengths = sb.key_lengths().data();

		m3bp::internal::OutputBufferImpl buffer_impl;
		buffer_impl.bind_fragment(std::move(sb));
		auto buffer =
			m3bp::internal::OutputBufferImpl::wrap_impl(
				std::move(buffer_impl));

		EXPECT_EQ(data_buffer,      buffer.data_buffer());
		EXPECT_EQ(record_count,     buffer.max_record_count());
		EXPECT_EQ(data_buffer_size, buffer.data_buffer_size());
		EXPECT_EQ(offset_table,     buffer.offset_table());
		EXPECT_EQ(key_lengths,      buffer.key_length_table());
	}
	EXPECT_EQ(0u, memory_manager->total_memory_usage());
}

TEST(OutputBuffer, Unbinded){
	m3bp::OutputBuffer buffer;
	EXPECT_EQ(0u,      buffer.max_record_count());
	EXPECT_EQ(0u,      buffer.data_buffer_size());
	EXPECT_EQ(nullptr, buffer.data_buffer());
	EXPECT_EQ(nullptr, buffer.offset_table());
	EXPECT_EQ(nullptr, buffer.key_length_table());
}

