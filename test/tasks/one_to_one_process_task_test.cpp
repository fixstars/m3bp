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
#include "tasks/process/one_to_one_process_logical_task.hpp"
#include "util/generator_util.hpp"
#include "util/sender_task.hpp"
#include "util/receiver_task.hpp"
#include "util/execution_util.hpp"
#include "util/processors/unite_processor.hpp"
#include "util/processors/hash_join_processor.hpp"
#include "util/workloads/unite.hpp"
#include "util/workloads/hash_join.hpp"

namespace {

template <typename T>
void run_unite_test(util::workloads::UniteWorkload<T> workload){
	using Processor = util::processors::TestUniteProcessor<T>;
	const int concurrency = 4;
	const auto input0 = workload.input0();
	const auto input1 = workload.input1();

	m3bp::LogicalGraph graph;
	auto receiver = std::make_shared<util::ReceiverTask<T>>(1);
	const auto sender0_id = graph.add_logical_task(
		std::make_shared<util::SenderTask<T>>(input0.begin(), input0.end()));
	const auto sender1_id = graph.add_logical_task(
		std::make_shared<util::SenderTask<T>>(input1.begin(), input1.end()));
	const auto processor_id = graph.add_logical_task(
		std::make_shared<m3bp::OneToOneProcessLogicalTask>(
			m3bp::internal::ProcessorWrapper(Processor()), concurrency));
	const auto receiver_id = graph.add_logical_task(receiver);
	graph
		.add_edge(
			m3bp::LogicalGraph::Port(sender0_id, 0),
			m3bp::LogicalGraph::Port(processor_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER)
		.add_edge(
			m3bp::LogicalGraph::Port(sender1_id, 0),
			m3bp::LogicalGraph::Port(processor_id, 1),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER)
		.add_edge(
			m3bp::LogicalGraph::Port(processor_id, 0),
			m3bp::LogicalGraph::Port(receiver_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER);
	util::execute_logical_graph(graph, concurrency);

	workload.verify(receiver->received_data(0));
}

}

TEST(OneToOneProcessTask, UniteZeroBuffers){
	run_unite_test(util::workloads::UniteWorkload<int>(0, 100));
}

TEST(OneToOneProcessTask, UniteEmptyBuffers){
	run_unite_test(util::workloads::UniteWorkload<int>(7, 0));
}

TEST(OneToOneProcessTask, Unite){
	run_unite_test(util::workloads::UniteWorkload<std::string>(11, 110));
}


namespace {

template <typename K, typename V0, typename V1>
void run_hash_join_test(
	util::workloads::HashJoinWorkload<K, V0, V1> workload)
{
	using Processor =
		util::processors::TestHashJoinProcessor<K, V0, V1>;
	using ResultType = std::pair<K, std::pair<V0, V1>>;
	const int concurrency = 4;
	const auto input0 = workload.input0();
	const auto input1 = workload.input1();

	m3bp::LogicalGraph graph;
	auto receiver = std::make_shared<util::ReceiverTask<ResultType>>(1);
	const auto sender0_id = graph.add_logical_task(
		std::make_shared<util::SenderTask<std::pair<K, V0>>>(
			input0.begin(), input0.end()));
	const auto sender1_id = graph.add_logical_task(
		std::make_shared<util::SenderTask<std::pair<K, V1>>>(
			input1.begin(), input1.end()));
	const auto processor_id = graph.add_logical_task(
		std::make_shared<m3bp::OneToOneProcessLogicalTask>(
			m3bp::internal::ProcessorWrapper(Processor()), concurrency));
	const auto receiver_id = graph.add_logical_task(receiver);
	graph
		.add_edge(
			m3bp::LogicalGraph::Port(sender0_id, 0),
			m3bp::LogicalGraph::Port(processor_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::ENTRY)
		.add_edge(
			m3bp::LogicalGraph::Port(sender1_id, 0),
			m3bp::LogicalGraph::Port(processor_id, 1),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER)
		.add_edge(
			m3bp::LogicalGraph::Port(processor_id, 0),
			m3bp::LogicalGraph::Port(receiver_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER);
	util::execute_logical_graph(graph, concurrency);

	workload.verify(receiver->received_data(0));
}

}

TEST(OneToOneProcessTask, HashJoinEmptyBroadcast){
	run_hash_join_test(util::workloads::HashJoinWorkload<int, int, int>(
		0, 4, 100));
}

TEST(OneToOneProcessTask, HashJoinEmptyInput){
	run_hash_join_test(util::workloads::HashJoinWorkload<int, int, int>(
		100, 0, 0));
}

TEST(OneToOneProcessTask, HashJoin){
	run_hash_join_test(
		util::workloads::HashJoinWorkload<std::string, int, std::string>(
			100, 9, 200));
}

