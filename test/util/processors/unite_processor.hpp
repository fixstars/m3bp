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
#ifndef M3BP_TEST_UTIL_PROCESSORS_UNITE_PROCESSOR_HPP
#define M3BP_TEST_UTIL_PROCESSORS_UNITE_PROCESSOR_HPP

#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include "m3bp/processor_base.hpp"
#include "m3bp/task.hpp"
#include "util/generator_util.hpp"
#include "util/processors/test_processor_base.hpp"
#include "util/processors/input_deserializer.hpp"
#include "util/processors/output_builder.hpp"

namespace util {
namespace processors {

template <typename T>
class TestUniteProcessor : public TestProcessorBase {

public:
	TestUniteProcessor()
		: TestProcessorBase(
			{
				m3bp::InputPort("input0")
					.movement(m3bp::Movement::ONE_TO_ONE),
				m3bp::InputPort("input1")
					.movement(m3bp::Movement::ONE_TO_ONE)
			},
			{
				m3bp::OutputPort("output0")
			})
	{ }

	virtual void run(m3bp::Task &task) override {
		TestProcessorBase::run(task);
		for(int i = 0; i < 2; ++i){
			auto reader = task.input(i);
			const auto received = deserialize_value_only_buffer<T>(reader);
			auto writer = task.output(0);
			auto output = build_output_buffer(writer, received);
			writer.flush_buffer(std::move(output), received.size());
		}
	}

};

}
}

#endif
