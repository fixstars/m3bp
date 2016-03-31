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
#ifndef M3BP_TEST_UTIL_PROCESSORS_REDUCE_BY_KEY_PROCESSOR_HPP
#define M3BP_TEST_UTIL_PROCESSORS_REDUCE_BY_KEY_PROCESSOR_HPP

#include <gtest/gtest.h>
#include <vector>
#include <numeric>
#include "m3bp/processor_base.hpp"
#include "m3bp/task.hpp"
#include "util/generator_util.hpp"
#include "util/processors/test_processor_base.hpp"
#include "util/processors/input_deserializer.hpp"
#include "util/processors/output_builder.hpp"

namespace util {
namespace processors {

template <typename KeyType, typename ValueType>
class TestReduceByKeyProcessor : public TestProcessorBase {

public:
	TestReduceByKeyProcessor()
		: TestProcessorBase(
			{
				m3bp::InputPort("input0")
					.movement(m3bp::Movement::SCATTER_GATHER)
			},
			{
				m3bp::OutputPort("output0")
					.has_key(true)
			})
	{ }

	virtual void run(m3bp::Task &task) override {
		using PairType = std::pair<KeyType, ValueType>;
		TestProcessorBase::run(task);
		auto reader = task.input(0);
		const auto received =
			deserialize_grouped_buffer<KeyType, ValueType>(reader);
		std::vector<PairType> result;
		for(const auto &g : received){
			result.emplace_back(g.first, std::accumulate(
				g.second.begin(), g.second.end(), ValueType()));
		}
		auto writer = task.output(0);
		auto output = build_output_buffer(writer, result);
		writer.flush_buffer(std::move(output), result.size());
	}

};

}
}

#endif
