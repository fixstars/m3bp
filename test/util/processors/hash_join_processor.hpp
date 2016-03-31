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
#ifndef M3BP_TEST_UTIL_PROCESSORS_HASH_JOIN_PROCESSOR_HPP
#define M3BP_TEST_UTIL_PROCESSORS_HASH_JOIN_PROCESSOR_HPP

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

template <typename KeyType, typename Value0Type, typename Value1Type>
class TestHashJoinProcessor : public TestProcessorBase {

private:
	using Pair0Type = std::pair<KeyType, Value0Type>;
	using Pair1Type = std::pair<KeyType, Value1Type>;
	using JoinedType =
		std::pair<KeyType, std::pair<Value0Type, Value1Type>>;

	std::unordered_map<KeyType, Value0Type> m_hashmap;

public:
	TestHashJoinProcessor()
		: TestProcessorBase(
			{
				m3bp::InputPort("input0")
					.movement(m3bp::Movement::BROADCAST),
				m3bp::InputPort("input1")
					.movement(m3bp::Movement::ONE_TO_ONE)
			},
			{
				m3bp::OutputPort("output0")
					.has_key(true)
			})
		, m_hashmap()
	{ }

	virtual void global_initialize(m3bp::Task &task) override {
		TestProcessorBase::global_initialize(task);
		auto reader = task.input(0);
		const auto received =
			deserialize_value_only_buffer<Pair0Type>(reader);
		for(auto &p : received){ m_hashmap.emplace(std::move(p)); }
	}

	virtual void global_finalize(m3bp::Task &task) override {
		TestProcessorBase::global_finalize(task);
		m_hashmap = std::unordered_map<KeyType, Value0Type>();
	}

	virtual void run(m3bp::Task &task) override {
		TestProcessorBase::run(task);
		auto reader = task.input(1);
		const auto received =
			deserialize_value_only_buffer<Pair1Type>(reader);

		std::vector<JoinedType> joined;
		for(const auto &p : received){
			const auto it = m_hashmap.find(p.first);
			if(it == m_hashmap.end()){ continue; }
			joined.emplace_back(
				p.first, std::make_pair(it->second, p.second));
		}
		auto writer = task.output(0);
		auto output = build_output_buffer(writer, joined);
		writer.flush_buffer(std::move(output), joined.size());
	}

};

}
}

#endif
