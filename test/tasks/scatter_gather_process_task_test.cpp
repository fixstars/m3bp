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
#include "tasks/shuffle/shuffle_logical_task.hpp"
#include "tasks/process/scatter_gather_process_logical_task.hpp"
#include "util/generator_util.hpp"
#include "util/sender_task.hpp"
#include "util/receiver_task.hpp"
#include "util/execution_util.hpp"
#include "util/processors/reduce_by_key_processor.hpp"
#include "util/processors/sort_join_processor.hpp"
#include "util/workloads/reduce_by_key.hpp"
#include "util/workloads/sort_join.hpp"

namespace {

template <typename KeyType, typename ValueType>
void run_reduce_by_key_test(
	m3bp::size_type partition_count,
	util::workloads::ReduceByKeyWorkload<KeyType, ValueType> workload)
{
	using PairType = std::pair<KeyType, ValueType>;
	using Processor =
		util::processors::TestReduceByKeyProcessor<KeyType, ValueType>;
	const int concurrency = 4;
	const auto input = workload.input();

	m3bp::LogicalGraph graph;
	auto receiver = std::make_shared<util::ReceiverTask<PairType>>(1);
	const auto sender_id = graph.add_logical_task(
		std::make_shared<util::SenderTask<PairType>>(
			input.begin(), input.end()));
	const auto shuffler_id = graph.add_logical_task(
		std::make_shared<m3bp::ShuffleLogicalTask>(partition_count));
	const auto processor_id = graph.add_logical_task(
		std::make_shared<m3bp::ScatterGatherProcessLogicalTask>(
			m3bp::internal::ProcessorWrapper(Processor()),
			concurrency,
			partition_count));
	const auto receiver_id = graph.add_logical_task(receiver);
	graph
		.add_edge(
			m3bp::LogicalGraph::Port(sender_id, 0),
			m3bp::LogicalGraph::Port(shuffler_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER)
		.add_edge(
			m3bp::LogicalGraph::Port(shuffler_id, 0),
			m3bp::LogicalGraph::Port(processor_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER)
		.add_edge(
			m3bp::LogicalGraph::Port(processor_id, 0),
			m3bp::LogicalGraph::Port(receiver_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER);
	util::execute_logical_graph(graph, concurrency);

	workload.verify(receiver->received_data(0));
}

}

TEST(ScatterGatherProcessTask, ReduceByKeyZeroBuffers){
	run_reduce_by_key_test<int, int>(
		10, util::workloads::ReduceByKeyWorkload<int, int>(10, 0, 0));
}

TEST(ScatterGatherProcessTask, ReduceByKeyEmptyBuffers){
	run_reduce_by_key_test<int, int>(
		10, util::workloads::ReduceByKeyWorkload<int, int>(10, 10, 0));
}

TEST(ScatterGatherProcessTask, ReduceByKeySmall){
	run_reduce_by_key_test<int, int>(
		4, util::workloads::ReduceByKeyWorkload<int, int>(20, 10, 100));
}

TEST(ScatterGatherProcessTask, ReduceByKeyLarge){
	run_reduce_by_key_test<std::string, int>(
		24,
		util::workloads::ReduceByKeyWorkload<std::string, int>(
			500, 100, 1000));
}


namespace {

template <typename K, typename V0, typename V1>
void run_sort_join_test(
	m3bp::size_type partition_count,
	util::workloads::SortJoinWorkload<K, V0, V1> workload)
{
	using JoinedType = std::pair<K, std::pair<V0, V1>>;
	using Processor = util::processors::TestSortJoinProcessor<K, V0, V1>;
	const int concurrency = 4;
	const auto input0 = workload.input0();
	const auto input1 = workload.input1();

	m3bp::LogicalGraph graph;
	auto receiver = std::make_shared<util::ReceiverTask<JoinedType>>(1);
	const auto sender0_id = graph.add_logical_task(
		std::make_shared<util::SenderTask<std::pair<K, V0>>>(
			input0.begin(), input0.end()));
	const auto sender1_id = graph.add_logical_task(
		std::make_shared<util::SenderTask<std::pair<K, V1>>>(
			input1.begin(), input1.end()));
	const auto shuffler0_id = graph.add_logical_task(
		std::make_shared<m3bp::ShuffleLogicalTask>(partition_count));
	const auto shuffler1_id = graph.add_logical_task(
		std::make_shared<m3bp::ShuffleLogicalTask>(partition_count));
	const auto processor_id = graph.add_logical_task(
		std::make_shared<m3bp::ScatterGatherProcessLogicalTask>(
			m3bp::internal::ProcessorWrapper(Processor()),
			concurrency,
			partition_count));
	const auto receiver_id = graph.add_logical_task(receiver);
	graph
		.add_edge(
			m3bp::LogicalGraph::Port(sender0_id, 0),
			m3bp::LogicalGraph::Port(shuffler0_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER)
		.add_edge(
			m3bp::LogicalGraph::Port(shuffler0_id, 0),
			m3bp::LogicalGraph::Port(processor_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER)
		.add_edge(
			m3bp::LogicalGraph::Port(sender1_id, 0),
			m3bp::LogicalGraph::Port(shuffler1_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER)
		.add_edge(
			m3bp::LogicalGraph::Port(shuffler1_id, 0),
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

TEST(ScatterGatherProcessTask, SortJoinZeroBuffers){
	run_sort_join_test(
		10, util::workloads::SortJoinWorkload<int, int, int>(10, 0, 0));
}

TEST(ScatterGatherProcessTask, SortJoinEmptyBuffers){
	run_sort_join_test(
		10, util::workloads::SortJoinWorkload<int, int, int>(10, 10, 0));
}

TEST(ScatterGatherProcessTask, SortJoinSmall){
	run_sort_join_test(
		4,
		util::workloads::SortJoinWorkload<int, int, std::string>(20, 10, 100));
}

TEST(ScatterGatherProcessTask, SortJoinLarge){
	run_sort_join_test(
		24,
		util::workloads::SortJoinWorkload<std::string, std::string, int>(
			100000, 100, 1000));
}

