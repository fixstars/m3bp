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
#include "memory/memory_manager.hpp"
#include "memory/memory_reference.hpp"

namespace {

void fill_memory(void *ptr, size_t size){
	uint8_t *ptr_u8 = reinterpret_cast<uint8_t *>(ptr);
	for(size_t i = 0; i < size; ++i){ ptr_u8[i] = 0; }
}

}

TEST(MemoryManager, SimpleAllocation){
	const size_t size = 1024;
	auto mm = std::make_shared<m3bp::MemoryManager>();
	EXPECT_EQ(0u, mm->total_memory_usage());
	{
		auto mr0 = mm->allocate(size);
		EXPECT_EQ(size, mm->total_memory_usage());
		{
			auto locked_mr0 = mr0.lock();
			EXPECT_NE(nullptr, locked_mr0.pointer());
			fill_memory(locked_mr0.pointer(), size);
		}
		auto mr1 = mr0;
		mr0 = m3bp::MemoryReference();
		EXPECT_EQ(size, mm->total_memory_usage());
	}
	EXPECT_EQ(0u, mm->total_memory_usage());
}

