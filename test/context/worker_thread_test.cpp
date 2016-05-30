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
#include <memory>
#include <atomic>
#include "m3bp/configuration.hpp"
#include "m3bp/flow_graph.hpp"
#include "m3bp/thread_observer_base.hpp"
#include "common/thread_specific.hpp"
#include "context/worker_thread.hpp"
#include "context/execution_context.hpp"
#include "scheduler/scheduler.hpp"

namespace {

#ifdef M3BP_NO_THREAD_LOCAL
m3bp::ThreadSpecific<int> g_ts_initialize_order_counter;
#else
static thread_local int g_thread_initialize_order_counter = 0;
#endif

template <
	bool THROW_ON_INITIALIZE,
	bool THROW_ON_FINALIZE>
class ThrowThreadObserver : public m3bp::ThreadObserverBase {
private:
	bool m_initialized;
	int  m_on_initialize_counter;
public:
	ThrowThreadObserver()
		: m_initialized(false)
		, m_on_initialize_counter(0)
	{ }
	~ThrowThreadObserver(){
		EXPECT_FALSE(m_initialized);
	}

	virtual void on_initialize() override {
		EXPECT_FALSE(m_initialized);
		if(THROW_ON_INITIALIZE){ throw std::exception(); }
#ifdef M3BP_NO_THREAD_LOCAL
		m_on_initialize_counter = g_ts_initialize_order_counter.get()++;
#else
		m_on_initialize_counter = g_thread_initialize_order_counter++;
#endif
		m_initialized = true;
	}
	virtual void on_finalize() override {
		EXPECT_TRUE(m_initialized);
		m_initialized = false;
#ifdef M3BP_NO_THREAD_LOCAL
		const auto counter = --g_ts_initialize_order_counter.get();
#else
		const auto counter = --g_thread_initialize_order_counter;
#endif
		EXPECT_EQ(m_on_initialize_counter, counter);
		if(THROW_ON_FINALIZE){ throw std::exception(); }
	}
};

class FailThreadObserver : public m3bp::ThreadObserverBase {
public:
	virtual void on_initialize() override { FAIL(); }
	virtual void on_finalize()   override { FAIL(); }
};

void run_observation_test(
	std::vector<std::shared_ptr<m3bp::ThreadObserverBase>> observers,
	bool expect_is_cancelled)
{
	auto config =
		m3bp::Configuration()
			.max_concurrency(1);
	m3bp::ExecutionContext ctx(config, m3bp::FlowGraph());

	m3bp::WorkerThread worker(0);
	worker.run(ctx, observers);
	worker.join();
	EXPECT_EQ(expect_is_cancelled, ctx.scheduler().is_cancelled());
}

}

TEST(WorkerThread, NormallyObservation){
	std::vector<std::shared_ptr<m3bp::ThreadObserverBase>> observers;
	observers.emplace_back(
		std::make_shared<ThrowThreadObserver<false, false>>());
	observers.emplace_back(
		std::make_shared<ThrowThreadObserver<false, false>>());
	observers.emplace_back(
		std::make_shared<ThrowThreadObserver<false, false>>());
	run_observation_test(std::move(observers), false);
}

TEST(WorkerThread, ThrowInInitializeObserver){
	std::vector<std::shared_ptr<m3bp::ThreadObserverBase>> observers;
	observers.emplace_back(
		std::make_shared<ThrowThreadObserver<false, false>>());
	observers.emplace_back(
		std::make_shared<ThrowThreadObserver<true, false>>());
	observers.emplace_back(std::make_shared<FailThreadObserver>());
	run_observation_test(std::move(observers), true);
}

TEST(WorkerThread, ThrowInFinalizeObserver){
	std::vector<std::shared_ptr<m3bp::ThreadObserverBase>> observers;
	observers.emplace_back(
		std::make_shared<ThrowThreadObserver<false, false>>());
	observers.emplace_back(
		std::make_shared<ThrowThreadObserver<false, true>>());
	observers.emplace_back(
		std::make_shared<ThrowThreadObserver<false, false>>());
	run_observation_test(std::move(observers), true);
}

