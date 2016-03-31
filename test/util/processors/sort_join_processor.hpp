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
#ifndef M3BP_TEST_UTIL_PROCESSORS_SORT_JOIN_PROCESSOR_HPP
#define M3BP_TEST_UTIL_PROCESSORS_SORT_JOIN_PROCESSOR_HPP

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

template <typename KeyType, typename Value0Type, typename Value1Type>
class TestSortJoinProcessor : public TestProcessorBase {

public:
	TestSortJoinProcessor()
		: TestProcessorBase(
			{
				m3bp::InputPort("input0")
					.movement(m3bp::Movement::SCATTER_GATHER),
				m3bp::InputPort("input1")
					.movement(m3bp::Movement::SCATTER_GATHER)
			},
			{
				m3bp::OutputPort("output0")
					.has_key(true)
			})
	{ }

	virtual void run(m3bp::Task &task) override {
		using JoinedType =
			std::pair<KeyType, std::pair<Value0Type, Value1Type>>;
		TestProcessorBase::run(task);

		auto reader0 = task.input(0), reader1 = task.input(1);
		const auto received0 =
			deserialize_grouped_buffer<KeyType, Value0Type>(reader0);
		const auto received1 =
			deserialize_grouped_buffer<KeyType, Value1Type>(reader1);

		std::vector<JoinedType> result;
		auto it0 = received0.begin();
		auto it1 = received1.begin();
		while(it0 != received0.end() && it1 != received1.end()){
			const int cmp = binary_compare(it0->first, it1->first);
			if(cmp < 0){
				++it0;
			}else if(cmp > 0){
				++it1;
			}else{
				for(const auto &x : it0->second){
					for(const auto &y : it1->second){
						result.emplace_back(it0->first, std::make_pair(x, y));
					}
				}
				++it0;
				++it1;
			}
		}

		auto writer = task.output(0);
		auto output = build_output_buffer(writer, result);
		writer.flush_buffer(std::move(output), result.size());
	}

};

}
}

#endif
