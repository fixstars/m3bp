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
#ifndef M3BP_TEST_UTIL_WORKLOADS_UNITE_HPP
#define M3BP_TEST_UTIL_WORKLOADS_UNITE_HPP

#include <gtest/gtest.h>
#include <vector>
#include <utility>
#include "util/generator_util.hpp"

namespace util {
namespace workloads {

template <typename T>
class UniteWorkload {

private:
	std::vector<std::vector<T>> m_input0;
	std::vector<std::vector<T>> m_input1;
	std::vector<T> m_expected;

public:
	UniteWorkload(
		m3bp::size_type fragment_count,
		m3bp::size_type records_per_fragment)
		: m_input0(fragment_count)
		, m_input1(records_per_fragment)
		, m_expected()
	{
		for(auto &v : m_input0){
			v = generate_random_sequence<T>(records_per_fragment);
			for(const auto &x : v){ m_expected.push_back(x); }
		}
		for(auto &v : m_input1){
			v = generate_random_sequence<T>(records_per_fragment);
			for(const auto &x : v){ m_expected.push_back(x); }
		}
		std::sort(m_expected.begin(), m_expected.end());
	}

	std::vector<std::vector<T>> input0() const {
		return m_input0;
	}
	std::vector<std::vector<T>> input1() const {
		return m_input1;
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

