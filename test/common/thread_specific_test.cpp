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
#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include "common/thread_specific.hpp"

namespace {

std::atomic<int> g_dtor_counter;

struct TestCounter {
	int value;
	TestCounter() : value(1) { }
	~TestCounter(){ g_dtor_counter += value; }
};

m3bp::ThreadSpecific<TestCounter> g_ts_test_counter;

}

TEST(ThreadSpecific, GetAndSet){
	const int num_threads = 4;
	std::vector<std::thread> threads(num_threads);
	g_dtor_counter.store(0);
	for(int i = 0; i < num_threads; ++i){
		threads[i] = std::thread([i](){
			EXPECT_EQ(1, g_ts_test_counter.get().value);
			g_ts_test_counter.get().value += i;
		});
	}
	for(int i = 0; i < num_threads; ++i){
		threads[i].join();
		threads[i] = std::thread();
	}
	const int expected = (num_threads + 1) * num_threads / 2;
	EXPECT_EQ(expected, g_dtor_counter.load());
}

