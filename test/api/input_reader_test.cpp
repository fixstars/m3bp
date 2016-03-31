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
#include "m3bp/input_reader.hpp"
#include "api/internal/input_reader_impl.hpp"
#include "memory/memory_manager.hpp"
#include "memory/memory_reference.hpp"
#include "memory/serialized_buffer.hpp"

TEST(InputReader, RawBuffer){
	auto memory_manager = std::make_shared<m3bp::MemoryManager>();
	{
		auto sb = m3bp::SerializedBuffer::allocate_grouped_buffer(
			*memory_manager, 200, 100, 1000, 2000);
		sb.record_count(180);
		auto mobj = sb.raw_reference();

		m3bp::internal::InputReaderImpl reader_impl;
		reader_impl.set_fragment(mobj);
		m3bp::InputReader reader =
			m3bp::internal::InputReaderImpl::wrap_impl(std::move(reader_impl));

		m3bp::InputBuffer ret_buffer = reader.raw_buffer();
		EXPECT_EQ(sb.keys_data(),             ret_buffer.key_buffer());
		EXPECT_EQ(sb.keys_offsets().data(),   ret_buffer.key_offset_table());
		EXPECT_EQ(sb.values_data(),           ret_buffer.value_buffer());
		EXPECT_EQ(sb.value_group_offsets().data(),
		          ret_buffer.value_offset_table());
	}
}

