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
#ifndef M3BP_TEST_UTIL_WORKLOADS_BROADCAST_DUPLICATION_HPP
#define M3BP_TEST_UTIL_WORKLOADS_BROADCAST_DUPLICATION_HPP

#include <gtest/gtest.h>
#include <vector>
#include "util/generator_util.hpp"

namespace util {
namespace workloads {

template <typename T>
class BroadcastDuplicationWorkload {

private:
	m3bp::size_type m_task_count;
	std::vector<std::vector<T>> m_input;
	std::vector<T> m_expected;

public:
	BroadcastDuplicationWorkload(
		m3bp::size_type task_count,
		m3bp::size_type input_record_count)
		: m_task_count(task_count)
		, m_input(1, std::vector<T>(input_record_count))
		, m_expected()
	{
		for(auto &x : m_input[0]){
			x = generate_random<T>();
			for(m3bp::size_type i = 0; i < task_count; ++i){
				m_expected.push_back(x);
			}
		}
		std::sort(m_expected.begin(), m_expected.end());
	}

	m3bp::size_type task_count() const {
		return m_task_count;
	}

	std::vector<std::vector<T>> input() const {
		return m_input;
	}

	bool verify(std::vector<T> actual) const {
		std::sort(actual.begin(), actual.end());
		EXPECT_EQ(m_expected, actual);
		return m_expected == actual;
	}

};

}
}

#endif

