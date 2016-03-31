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
#include "tasks/shuffle/shuffle_logical_task.hpp"
#include "graph/logical_graph.hpp"
#include "common/hash_function.hpp"
#include "util/generator_util.hpp"
#include "util/sender_task.hpp"
#include "util/receiver_task.hpp"
#include "util/execution_util.hpp"

namespace {

template <typename KeyType, typename ValueType>
void run_test(
	m3bp::size_type partition_count,
	m3bp::size_type fragment_count,
	m3bp::size_type record_count)
{
	using PairType = std::pair<KeyType, ValueType>;
	std::vector<std::vector<PairType>> dataset(fragment_count);
	for(m3bp::identifier_type i = 0; i < fragment_count; ++i){
		for(m3bp::identifier_type j = 0; j < record_count; ++j){
			dataset[i].emplace_back(
				util::generate_random<KeyType>(),
				util::generate_random<ValueType>());
		}
	}

	m3bp::LogicalGraph graph;
	auto receiver = std::make_shared<
		util::GroupedReceiverTask<KeyType, ValueType>>(partition_count);
	const auto sender_id = graph.add_logical_task(
		std::make_shared<util::SenderTask<PairType>>(
			dataset.begin(), dataset.end()));
	const auto shuffle_id = graph.add_logical_task(
		std::make_shared<m3bp::ShuffleLogicalTask>(partition_count));
	const auto receiver_id = graph.add_logical_task(receiver);
	graph
		.add_edge(
			m3bp::LogicalGraph::Port(sender_id, 0),
			m3bp::LogicalGraph::Port(shuffle_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER)
		.add_edge(
			m3bp::LogicalGraph::Port(shuffle_id, 0),
			m3bp::LogicalGraph::Port(receiver_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER);
	util::execute_logical_graph(graph);

	using BinaryPair = std::pair<std::vector<uint8_t>, PairType>;
	std::vector<std::vector<BinaryPair>> partitioned(partition_count);
	for(const auto &fragment : dataset){
		for(const auto &x : fragment){
			const auto len = util::binary_length(x.first);
			std::vector<uint8_t> buf(len);
			util::write_binary(buf.data(), x.first);
			const auto partition =
				util::compute_hash(x.first, partition_count);
			partitioned[partition].emplace_back(std::move(buf), x);
		}
	}

	using GroupType = std::pair<KeyType, std::vector<ValueType>>;
	for(m3bp::identifier_type i = 0; i < partition_count; ++i){
		std::vector<GroupType> expected_groups;
		std::sort(partitioned[i].begin(), partitioned[i].end());
		std::vector<uint8_t> last_key;
		for(const auto &x : partitioned[i]){
			if(x.first != last_key){
				last_key = x.first;
				expected_groups.emplace_back(
					x.second.first, std::vector<ValueType>());
			}
			expected_groups.back().second.emplace_back(x.second.second);
		}
		for(auto &g : expected_groups){
			std::sort(g.second.begin(), g.second.end());
		}
		std::vector<GroupType> actual_groups = receiver->received_data(i);
		for(auto &g : actual_groups){
			std::sort(g.second.begin(), g.second.end());
		}
		EXPECT_EQ(expected_groups, actual_groups);
	}
}

}

TEST(ShuffleTask, EmptyBuffer){
	run_test<int, int>(1, 0, 0);
}

TEST(ShuffleTask, EmptyFragment){
	run_test<int, int>(1, 1, 0);
}

TEST(ShuffleTask, RandomFixedKeyAndFixedValue){
	run_test<int, unsigned long long>(11, 7, 1000);
}

TEST(ShuffleTask, RandomFixedKeyAndVarLenValue){
	run_test<int, std::string>(16, 10, 1000);
}

TEST(ShuffleTask, RandomVarLenKeyAndFixedValue){
	run_test<std::string, int>(19, 16, 1000);
}

TEST(ShuffleTask, RandomVarLenKeyAndVarLenValue){
	run_test<std::string, std::string>(24, 19, 1000);
}

