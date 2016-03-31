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
#ifndef M3BP_TEST_UTIL_WORKLOADS_REDUCE_BY_KEY_HPP
#define M3BP_TEST_UTIL_WORKLOADS_REDUCE_BY_KEY_HPP

#include <gtest/gtest.h>
#include <vector>
#include <utility>
#include "util/generator_util.hpp"

namespace util {
namespace workloads {

template <typename KeyType, typename ValueType>
class ReduceByKeyWorkload {

public:
	using PairType = std::pair<KeyType, ValueType>;

private:
	std::vector<std::vector<PairType>> m_input;
	std::vector<PairType> m_expected;

public:
	ReduceByKeyWorkload(
		m3bp::size_type key_kinds,
		m3bp::size_type fragment_count,
		m3bp::size_type records_per_fragment)
		: m_input(fragment_count)
		, m_expected()
	{
		const auto keys =
			generate_distinct_random_sequence<KeyType>(key_kinds);
		std::uniform_int_distribution<> key_dist(0, key_kinds - 1);
		std::unordered_map<KeyType, ValueType> hashmap;
		for(auto &v : m_input){
			v = std::vector<PairType>(records_per_fragment);
			for(auto &p : v){
				p.first = keys[key_dist(g_random_engine)];
				p.second = generate_random<ValueType>();
				hashmap[p.first] += p.second;
			}
		}
		m_expected = std::vector<PairType>(hashmap.begin(), hashmap.end());
		std::sort(m_expected.begin(), m_expected.end());
	}

	std::vector<std::vector<PairType>> input() const {
		return m_input;
	}

	bool verify(std::vector<PairType> actual) const {
		std::sort(actual.begin(), actual.end());
		EXPECT_EQ(m_expected, actual);
		return m_expected == actual;
	}

};

}
}

#endif

