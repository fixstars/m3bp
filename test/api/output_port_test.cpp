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
#include "m3bp/output_port.hpp"

TEST(OutputPort, Parameters){
	m3bp::OutputPort oport("name");
	EXPECT_EQ("name", oport.name());
	EXPECT_FALSE(oport.has_key());
	EXPECT_EQ(&oport, &oport.has_key(true));
	EXPECT_TRUE(oport.has_key());
}

