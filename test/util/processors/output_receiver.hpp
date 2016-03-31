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
#ifndef M3BP_TEST_UTIL_PROCESSOR_OUTPUT_RECEIVER_HPP
#define M3BP_TEST_UTIL_PROCESSOR_OUTPUT_RECEIVER_HPP

#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include "m3bp/processor_base.hpp"
#include "m3bp/task.hpp"
#include "util/generator_util.hpp"
#include "util/processors/input_deserializer.hpp"
#include "util/processors/output_builder.hpp"
#include "util/processors/test_processor_base.hpp"

namespace util {
namespace processors {

template <typename T>
class TestOutputReceiver : public TestProcessorBase {

private:
	std::shared_ptr<std::vector<T>> m_received;

public:
	explicit TestOutputReceiver(
		std::shared_ptr<std::vector<T>> destination)
		: TestProcessorBase(
			{
				m3bp::InputPort("input0")
					.movement(m3bp::Movement::ONE_TO_ONE)
			},
			{ })
		, m_received(std::move(destination))
	{
		max_concurrency(1);
	}

	virtual void run(m3bp::Task &task) override {
		TestProcessorBase::run(task);

		auto reader = task.input(0);
		const auto received = deserialize_value_only_buffer<T>(reader);
		for(const auto &x : received){
			m_received->push_back(x);
		}
	}

};


template <typename KeyType, typename ValueType>
class TestGroupedOutputReceiver : public TestProcessorBase {

public:
	using GroupType = std::pair<KeyType, std::vector<ValueType>>;

private:
	std::shared_ptr<std::vector<GroupType>> m_received;

public:
	explicit TestGroupedOutputReceiver(
		std::shared_ptr<std::vector<GroupType>> destination)
		: TestProcessorBase(
			{
				m3bp::InputPort("input0")
					.movement(m3bp::Movement::SCATTER_GATHER)
			},
			{ })
		, m_received(std::move(destination))
	{
		max_concurrency(1);
	}

	virtual void run(m3bp::Task &task) override {
		TestProcessorBase::run(task);

		auto reader = task.input(0);
		auto received =
			deserialize_grouped_buffer<KeyType, ValueType>(reader);
		for(auto &x : received){
			m_received->push_back(std::move(x));
		}
	}

};

}
}

#endif

