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
#include <vector>
#include <algorithm>
#include "m3bp/processor_base.hpp"
#include "m3bp/task.hpp"
#include "m3bp/internal/processor_wrapper.hpp"
#include "tasks/process/input_process_logical_task.hpp"
#include "util/generator_util.hpp"
#include "util/sender_task.hpp"
#include "util/receiver_task.hpp"
#include "util/execution_util.hpp"
#include "util/processors/input_generator.hpp"
#include "util/processors/broadcast_duplicator.hpp"
#include "util/workloads/broadcast_duplication.hpp"

namespace {

template <typename T>
void run_no_input_test(m3bp::size_type task_count){
	using Processor = util::processors::TestInputGenerator<T>;
	const int concurrency = 4;

	std::vector<std::vector<T>> dataset;
	for(m3bp::identifier_type i = 0; i < task_count; ++i){
		const auto len = (util::generate_random<unsigned int>() % 30) + 10;
		std::vector<T> in_data(len);
		for(m3bp::identifier_type j = 0; j < len; ++j){
			in_data[j] = util::generate_random<T>();
		}
		dataset.emplace_back(std::move(in_data));
	}

	m3bp::LogicalGraph graph;
	auto receiver = std::make_shared<util::ReceiverTask<T>>(1);
	const auto processor_id = graph.add_logical_task(
		std::make_shared<m3bp::InputProcessLogicalTask>(
			m3bp::internal::ProcessorWrapper(Processor(dataset)),
			concurrency));
	const auto receiver_id = graph.add_logical_task(receiver);
	graph.add_edge(
		m3bp::LogicalGraph::Port(processor_id, 0),
		m3bp::LogicalGraph::Port(receiver_id, 0),
		m3bp::LogicalGraph::PhysicalSuccessor::BARRIER);
	util::execute_logical_graph(graph, concurrency);

	std::vector<T> expected;
	for(const auto &v : dataset){
		for(const auto &x : v){ expected.emplace_back(x); }
	}
	auto actual = receiver->received_data(0);
	std::sort(expected.begin(), expected.end());
	std::sort(actual.begin(), actual.end());
	EXPECT_EQ(expected, actual);
}

}

TEST(InputProcessTask, NoInputEmptyBuffer){
	run_no_input_test<int>(0);
}

TEST(InputProcessTask, NoInput){
	run_no_input_test<int>(10);
	run_no_input_test<std::string>(7);
}


namespace {

template <typename T>
void run_broadcast_only_test(
	util::workloads::BroadcastDuplicationWorkload<T> workload)
{
	using Processor = util::processors::TestBroadcastDuplicator<T>;
	const int concurrency = 4;
	const auto input = workload.input();

	m3bp::LogicalGraph graph;
	auto receiver = std::make_shared<util::ReceiverTask<T>>(1);
	const auto sender_id = graph.add_logical_task(
		std::make_shared<util::SenderTask<T>>(input.begin(), input.end()));
	const auto processor_id = graph.add_logical_task(
		std::make_shared<m3bp::InputProcessLogicalTask>(
			m3bp::internal::ProcessorWrapper(Processor(workload.task_count())),
			concurrency));
	const auto receiver_id = graph.add_logical_task(receiver);
	graph
		.add_edge(
			m3bp::LogicalGraph::Port(sender_id, 0),
			m3bp::LogicalGraph::Port(processor_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::ENTRY)
		.add_edge(
			m3bp::LogicalGraph::Port(processor_id, 0),
			m3bp::LogicalGraph::Port(receiver_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER);
	util::execute_logical_graph(graph, concurrency);

	workload.verify(receiver->received_data(0));
}

}

TEST(InputProcessTask, BroadcastDuplicator){
	run_broadcast_only_test(
		util::workloads::BroadcastDuplicationWorkload<std::string>(8, 66));
}

