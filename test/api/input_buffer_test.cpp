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
#include "m3bp/input_buffer.hpp"
#include "api/internal/input_buffer_impl.hpp"
#include "memory/memory_manager.hpp"
#include "memory/memory_reference.hpp"
#include "memory/serialized_buffer.hpp"

TEST(InputBuffer, OneToOneParameters){
	auto memory_manager = std::make_shared<m3bp::MemoryManager>();
	{
		auto sb = m3bp::SerializedBuffer::allocate_value_only_buffer(
			*memory_manager, 100, 1000);
		sb.record_count(90);
		auto mobj = sb.raw_reference();

		m3bp::internal::InputBufferImpl buffer_impl;
		buffer_impl.bind(mobj);
		auto buffer = m3bp::internal::InputBufferImpl::wrap_impl(
			std::move(buffer_impl));

		EXPECT_EQ(90u,                        buffer.record_count());
		EXPECT_EQ(sb.values_data(),           buffer.key_buffer());
		EXPECT_EQ(sb.values_offsets().data(), buffer.key_offset_table());
	}
}

TEST(InputBuffer, KeyValueParameters){
	auto memory_manager = std::make_shared<m3bp::MemoryManager>();
	{
		auto sb = m3bp::SerializedBuffer::allocate_grouped_buffer(
			*memory_manager, 200, 100, 1000, 2000);
		sb.record_count(180);
		auto mobj = sb.raw_reference();

		m3bp::internal::InputBufferImpl buffer_impl;
		buffer_impl.bind(mobj);
		auto buffer = m3bp::internal::InputBufferImpl::wrap_impl(
			std::move(buffer_impl));

		EXPECT_EQ(100u,                            buffer.record_count());
		EXPECT_EQ(sb.keys_data(),                  buffer.key_buffer());
		EXPECT_EQ(sb.keys_offsets().data(),        buffer.key_offset_table());
		EXPECT_EQ(sb.values_data(),                buffer.value_buffer());
		EXPECT_EQ(sb.value_group_offsets().data(), buffer.value_offset_table());
	}
}

TEST(InputBuffer, Unbinded){
	m3bp::internal::InputBufferImpl buffer_impl;
	m3bp::InputBuffer buffer =
		m3bp::internal::InputBufferImpl::wrap_impl(std::move(buffer_impl));

	EXPECT_EQ(0u, buffer.record_count());
	EXPECT_EQ(0u, buffer.key_offset_table()[0]);
}

