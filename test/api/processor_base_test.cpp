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
#include "m3bp/processor_base.hpp"
#include "m3bp/exception.hpp"

namespace {

class TestMixedProcessor : public m3bp::ProcessorBase {
public:
	TestMixedProcessor()
		: m3bp::ProcessorBase(
			{
				m3bp::InputPort("input0")
					.movement(m3bp::Movement::ONE_TO_ONE),
				m3bp::InputPort("input1")
					.movement(m3bp::Movement::SCATTER_GATHER),
			},
			{ })
	{ }
	virtual void run(m3bp::Task &) override { }
};

}

TEST(ProcessorBase, InvalidProcessor){
	EXPECT_THROW(TestMixedProcessor(), m3bp::ProcessorDefinitionError);
}

