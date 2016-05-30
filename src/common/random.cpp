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
#include <random>
#include "common/random.hpp"

#ifdef M3BP_NO_THREAD_LOCAL
#include "common/thread_specific.hpp"
#endif

namespace m3bp {

#ifdef M3BP_NO_THREAD_LOCAL
struct RandomEngine {
	std::random_device random_device;
	std::default_random_engine engine;

	RandomEngine()
		: random_device()
		, engine(random_device())
	{ }
};
static ThreadSpecific<RandomEngine> g_ts_random_engine;
#else
static thread_local std::random_device g_random_device;
static thread_local std::default_random_engine g_engine(g_random_device());
#endif

template <typename T>
T uniform_random_integer(T lo, T hi){
	std::uniform_int_distribution<T> dist(lo, hi);
#ifdef M3BP_NO_THREAD_LOCAL
	return dist(g_ts_random_engine.get().engine);
#else
	return dist(g_engine);
#endif
}

template          char uniform_random_integer(         char,          char);
template   signed char uniform_random_integer(  signed char,   signed char);
template unsigned char uniform_random_integer(unsigned char, unsigned char);

template          short uniform_random_integer(         short,          short);
template unsigned short uniform_random_integer(unsigned short, unsigned short);

template          long uniform_random_integer(         long,          long);
template unsigned long uniform_random_integer(unsigned long, unsigned long);

template          int uniform_random_integer(         int,          int);
template unsigned int uniform_random_integer(unsigned int, unsigned int);

template          long long
uniform_random_integer(         long long,          long long);
template unsigned long long
uniform_random_integer(unsigned long long, unsigned long long);

}

