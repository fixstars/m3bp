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
#ifndef M3BP_TEST_UTIL_GENERATOR_UTIL_HPP
#define M3BP_TEST_UTIL_GENERATOR_UTIL_HPP

#include <unordered_set>
#include <random>
#include "m3bp/types.hpp"

namespace util {

extern std::mt19937 g_random_engine;

template <typename T>
inline T generate_random(){
	std::uniform_int_distribution<T> dist;
	return dist(g_random_engine);
}

template <>
inline std::string generate_random(){
	static const int RANDOM_STRING_MAX_LENGTH = 10;
	std::uniform_int_distribution<int> len_dist(1, RANDOM_STRING_MAX_LENGTH);
	const int n = len_dist(g_random_engine);
	std::uniform_int_distribution<int> char_dist('a', 'z');
	std::string s(n, ' ');
	for(int i = 0; i < n; ++i){
		s[i] = char_dist(g_random_engine);
	}
	return s;
}

template <typename T>
inline std::vector<T> generate_random_sequence(m3bp::size_type n){
	std::vector<T> result(n);
	for(m3bp::size_type i = 0; i < n; ++i){
		result[i] = generate_random<T>();
	}
	return result;
}

template <typename T>
inline std::vector<T> generate_distinct_random_sequence(m3bp::size_type n){
	std::unordered_set<T> occur;
	std::vector<T> result;
	for(m3bp::size_type i = 0; i < n; ++i){
		auto x = generate_random<T>();
		while(occur.find(x) != occur.end()){
			x = generate_random<T>();
		}
		result.push_back(x);
		occur.insert(x);
	}
	return result;
}

}

#endif

