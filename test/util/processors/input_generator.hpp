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
#ifndef M3BP_TEST_UTIL_PROCESSOR_INPUT_GENERATOR_HPP
#define M3BP_TEST_UTIL_PROCESSOR_INPUT_GENERATOR_HPP

#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include "m3bp/processor_base.hpp"
#include "m3bp/task.hpp"
#include "util/generator_util.hpp"
#include "util/processors/output_builder.hpp"
#include "util/processors/test_processor_base.hpp"

namespace util {
namespace processors {

template <typename T>
class TestInputGenerator : public TestProcessorBase {

private:
	std::vector<std::vector<T>> m_dataset;
	std::vector<int> m_ran_flags;

public:
	explicit TestInputGenerator(
		std::vector<std::vector<T>> dataset)
		: TestProcessorBase(
			{ },
			{
				m3bp::OutputPort("output0")
					.has_key(is_pair<T>::value)
			})
		, m_dataset(std::move(dataset))
		, m_ran_flags(m_dataset.size())
	{ }

	virtual void global_initialize(m3bp::Task &task) override {
		TestProcessorBase::global_initialize(task);
		task_count(m_ran_flags.size());
		std::fill(m_ran_flags.begin(), m_ran_flags.end(), false);
	}

	virtual void global_finalize(m3bp::Task &task) override {
		TestProcessorBase::global_finalize(task);
		for(const auto &flag : m_ran_flags){
			EXPECT_TRUE(flag);
		}
	}

	virtual void run(m3bp::Task &task) override {
		TestProcessorBase::run(task);

		const auto task_id = task.physical_task_id();
		EXPECT_FALSE(m_ran_flags[task_id]);
		m_ran_flags[task_id] = true;

		auto writer = task.output(0);
		auto buffer = build_output_buffer(writer, m_dataset[task_id]);
		writer.flush_buffer(std::move(buffer), m_dataset[task_id].size());
	}

};

}
}

#endif

