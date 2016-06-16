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

class OutputWriterTestProcessor : public m3bp::ProcessorBase {
private:
	m3bp::size_type m_default_buffer_size;
	m3bp::size_type m_default_records_per_buffer;
public:
	OutputWriterTestProcessor(
		m3bp::size_type default_buffer_size,
		m3bp::size_type default_records_per_buffer)
		: m3bp::ProcessorBase(
			{ }, { m3bp::OutputPort("output") })
		, m_default_buffer_size(default_buffer_size)
		, m_default_records_per_buffer(default_records_per_buffer)
	{
		task_count(4);
	}
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
	virtual void run(m3bp::Task &task) override {
		auto writer = task.output(0);
		{
			auto buffer = writer.allocate_buffer();
			EXPECT_LE(m_default_buffer_size, buffer.data_buffer_size());
			EXPECT_LE(m_default_records_per_buffer, buffer.max_record_count());
		}
		{
			auto buffer = writer.allocate_buffer(0, 0);
			EXPECT_LE(m_default_buffer_size, buffer.data_buffer_size());
			EXPECT_LE(m_default_records_per_buffer, buffer.max_record_count());
		}
	}
};

}

TEST(TaskConfiguration, DefaultOutputBufferConfiguration){
	const int concurrency = 4;
	const m3bp::size_type default_buffer_size = 87654321;
	const m3bp::size_type default_records_per_buffer = 654321;
	m3bp::FlowGraph graph;
	graph.add_vertex("v0", OutputWriterTestProcessor(
		default_buffer_size, default_records_per_buffer));
	m3bp::Context ctx;
	ctx.set_configuration(
		m3bp::Configuration()
			.max_concurrency(concurrency)
			.default_output_buffer_size(default_buffer_size)
			.default_records_per_buffer(default_records_per_buffer));
	ctx.set_flow_graph(graph);
	ctx.execute();
	ctx.wait();
}

