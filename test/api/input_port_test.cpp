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
#include "m3bp/input_port.hpp"

TEST(InputPort, Parameters){
	m3bp::InputPort iport("name");
	EXPECT_EQ("name", iport.name());
	EXPECT_EQ(m3bp::Movement::UNDEFINED, iport.movement());
	EXPECT_FALSE(iport.value_comparator());
	EXPECT_EQ(&iport, &iport.movement(m3bp::Movement::SCATTER_GATHER));
	EXPECT_EQ(m3bp::Movement::SCATTER_GATHER, iport.movement());
	int value = 0;
	auto f = [&](const void *, const void *) -> bool {
		value = 100;
		return false;
	};
	EXPECT_EQ(&iport, &iport.value_comparator(f));
	iport.value_comparator()(nullptr, nullptr);
	EXPECT_EQ(100, value);
}

