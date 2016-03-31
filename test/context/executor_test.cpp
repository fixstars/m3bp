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
#include <mutex>
#include <memory>
#include <unordered_set>
#include "m3bp/processor_base.hpp"
#include "m3bp/context.hpp"
#include "m3bp/flow_graph.hpp"
#include "m3bp/task.hpp"
#include "m3bp/configuration.hpp"

namespace {

class TestInputProcessor : public m3bp::ProcessorBase {
public:
	TestInputProcessor()
		: m3bp::ProcessorBase(
			{ }, { m3bp::OutputPort("output") })
	{ }
	virtual void global_initialize(m3bp::Task &) override {
	}
	virtual void global_finalize(m3bp::Task &task) override {
		EXPECT_FALSE(task.is_cancelled());
	}
	virtual void thread_local_initialize(m3bp::Task &) override {
	}
	virtual void thread_local_finalize(m3bp::Task &task) override {
		EXPECT_FALSE(task.is_cancelled());
	}
	virtual void run(m3bp::Task &) override {
	}
};

class TestThrowProcessor : public m3bp::ProcessorBase {
private:
	static const m3bp::size_type TASK_COUNT = 4;
	std::shared_ptr<std::mutex> m_mutex;
	std::unordered_set<std::thread::id> m_initialized_threads;
public:
	TestThrowProcessor()
		: m3bp::ProcessorBase(
			{ m3bp::InputPort("input").movement(m3bp::Movement::BROADCAST) },
			{ m3bp::OutputPort("output") })
		, m_mutex(std::make_shared<std::mutex>())
	{
		task_count(TASK_COUNT);
	}
	virtual void global_initialize(m3bp::Task &) override {
	}
	virtual void global_finalize(m3bp::Task &task) override {
		EXPECT_TRUE(task.is_cancelled());
	}
	virtual void thread_local_initialize(m3bp::Task &) override {
		std::lock_guard<std::mutex> lock(*m_mutex);
		m_initialized_threads.insert(std::this_thread::get_id());
	}
	virtual void thread_local_finalize(m3bp::Task &task) override {
		std::lock_guard<std::mutex> lock(*m_mutex);
		EXPECT_NE(
			m_initialized_threads.end(),
			m_initialized_threads.find(std::this_thread::get_id()));
		EXPECT_TRUE(task.is_cancelled());
	}
	virtual void run(m3bp::Task &task) override {
		if(task.physical_task_id() == TASK_COUNT / 2){
			throw std::runtime_error("");
		}
	}
};

class TestFailOutputProcessor : public m3bp::ProcessorBase {
public:
	TestFailOutputProcessor()
		: m3bp::ProcessorBase(
			{ m3bp::InputPort("input").movement(m3bp::Movement::BROADCAST) },
			{ })
	{
		task_count(1);
	}
	virtual void global_initialize      (m3bp::Task &) override { FAIL(); }
	virtual void global_finalize        (m3bp::Task &) override { FAIL(); }
	virtual void thread_local_initialize(m3bp::Task &) override { FAIL(); }
	virtual void thread_local_finalize  (m3bp::Task &) override { FAIL(); }
	virtual void run                    (m3bp::Task &) override { FAIL(); }
};

}

TEST(Executor, Cancellation){
	const int concurrency = 4;
	m3bp::FlowGraph graph;
	auto v0 = graph.add_vertex("v0", TestInputProcessor());
	auto v1 = graph.add_vertex("v1", TestThrowProcessor());
	auto v2 = graph.add_vertex("v2", TestFailOutputProcessor());
	graph
		.add_edge(v0.output_port(0), v1.input_port(0))
		.add_edge(v1.output_port(0), v2.input_port(0));
	m3bp::Context ctx;
	ctx.set_configuration(
		m3bp::Configuration().max_concurrency(concurrency));
	ctx.set_flow_graph(graph);
	ctx.execute();
	EXPECT_THROW(ctx.wait(), std::runtime_error);
}

