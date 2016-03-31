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
#include "tasks/gather/gather_logical_task.hpp"
#include "graph/logical_graph.hpp"
#include "util/generator_util.hpp"
#include "util/sender_task.hpp"
#include "util/receiver_task.hpp"
#include "util/execution_util.hpp"

namespace {

template <typename T>
void run_test(m3bp::size_type fragment_count){
	std::vector<std::vector<T>> dataset;
	for(m3bp::identifier_type i = 0; i < fragment_count; ++i){
		const m3bp::size_type len =
			(util::generate_random<unsigned int>() % 30) + 10;
		std::vector<T> in_data(len);
		for(m3bp::identifier_type j = 0; j < len; ++j){
			in_data[j] = util::generate_random<T>();
		}
		dataset.emplace_back(std::move(in_data));
	}

	m3bp::LogicalGraph graph;
	auto receiver = std::make_shared<util::ReceiverTask<T>>(1);
	const auto sender_id = graph.add_logical_task(
		std::make_shared<util::SenderTask<T>>(
			dataset.begin(), dataset.end()));
	const auto gather_id = graph.add_logical_task(
		std::make_shared<m3bp::GatherLogicalTask>());
	const auto receiver_id = graph.add_logical_task(receiver);
	graph
		.add_edge(
			m3bp::LogicalGraph::Port(sender_id, 0),
			m3bp::LogicalGraph::Port(gather_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER)
		.add_edge(
			m3bp::LogicalGraph::Port(gather_id, 0),
			m3bp::LogicalGraph::Port(receiver_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER);
	util::execute_logical_graph(graph);

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

TEST(GatherTask, EmptyBuffer){
	run_test<int>(0);
}

TEST(GatherTask, SingleFragment){
	run_test<int>(1);
}

TEST(GatherTask, SingleVarLenFragment){
	run_test<std::string>(1);
}

TEST(GatherTask, MultipleFragments){
	run_test<int>(31);
}

TEST(GatherTask, MultipleVarLenFragments){
	run_test<std::string>(31);
}

