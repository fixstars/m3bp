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
#ifndef M3BP_TEST_UTIL_WORKLOADS_SORT_JOIN_HPP
#define M3BP_TEST_UTIL_WORKLOADS_SORT_JOIN_HPP

#include <gtest/gtest.h>
#include <vector>
#include <utility>
#include <unordered_map>
#include "util/generator_util.hpp"

namespace util {
namespace workloads {

template <typename KeyType, typename Value0Type, typename Value1Type>
class SortJoinWorkload {

public:
	using Input0Type = std::pair<KeyType, Value0Type>;
	using Input1Type = std::pair<KeyType, Value1Type>;
	using ResultType = std::pair<KeyType, std::pair<Value0Type, Value1Type>>;

private:
	std::vector<std::vector<Input0Type>> m_input0;
	std::vector<std::vector<Input1Type>> m_input1;
	std::vector<ResultType> m_expected;

public:
	SortJoinWorkload(
		m3bp::size_type key_kinds,
		m3bp::size_type fragment_count,
		m3bp::size_type records_per_fragment)
		: m_input0(fragment_count)
		, m_input1(fragment_count)
		, m_expected()
	{
		const auto keys =
			generate_distinct_random_sequence<KeyType>(key_kinds);
		std::uniform_int_distribution<> key_dist(0, key_kinds - 1);
		std::unordered_map<KeyType, std::vector<Value0Type>> hashmap;
		for(auto &v : m_input0){
			v = std::vector<Input0Type>(records_per_fragment);
			for(auto &p : v){
				p.first = keys[key_dist(g_random_engine)];
				p.second = generate_random<Value0Type>();
				hashmap[p.first].push_back(p.second);
			}
		}
		for(auto &v : m_input1){
			v = std::vector<Input1Type>(records_per_fragment);
			for(auto &p : v){
				p.first = keys[key_dist(g_random_engine)];
				p.second = generate_random<Value1Type>();
				for(const auto &q : hashmap[p.first]){
					m_expected.emplace_back(
						p.first, std::make_pair(q, p.second));
				}
			}
		}
		std::sort(m_expected.begin(), m_expected.end());
	}

	std::vector<std::vector<Input0Type>> input0() const {
		return m_input0;
	}
	std::vector<std::vector<Input1Type>> input1() const {
		return m_input1;
	}

	bool verify(std::vector<ResultType> actual) const {
		std::sort(actual.begin(), actual.end());
		EXPECT_EQ(m_expected, actual);
		return m_expected == actual;
	}

};

}
}

#endif

