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
#include "m3bp/types.hpp"
#include "scheduler/locality_option.hpp"
#include "tasks/shuffle/shuffle_logical_task.hpp"
#include "tasks/value_sort/value_sort_logical_task.hpp"
#include "util/execution_util.hpp"
#include "util/binary_util.hpp"
#include "util/generator_util.hpp"
#include "util/sender_task.hpp"
#include "util/receiver_task.hpp"

namespace {

template <typename KeyType, typename ValueType>
void run_value_sort_test(
	m3bp::size_type partition_count,
	m3bp::size_type key_count,
	m3bp::size_type fragment_per_port,
	m3bp::size_type record_per_fragment,
	std::function<bool(const void *, const void *)> comparator)
{
	using PairType = std::pair<KeyType, ValueType>;
	const int concurrency = 1;

	const auto keys =
		util::generate_distinct_random_sequence<KeyType>(key_count);
	std::vector<std::vector<PairType>> dataset;
	for(m3bp::identifier_type i = 0; i < fragment_per_port; ++i){
		const auto len = record_per_fragment;
		std::vector<PairType> fragment(len);
		for(auto &x : fragment){
			x.first = keys[util::generate_random<unsigned int>() % key_count];
			x.second = util::generate_random<ValueType>();
		}
		dataset.emplace_back(std::move(fragment));
	}

	m3bp::LogicalGraph graph;
	auto receiver = std::make_shared<
		util::GroupedReceiverTask<KeyType, ValueType>>(partition_count);
	const auto sender_id = graph.add_logical_task(
		std::make_shared<util::SenderTask<PairType>>(
			dataset.begin(), dataset.end()));
	const auto shuffler_id = graph.add_logical_task(
		std::make_shared<m3bp::ShuffleLogicalTask>(partition_count));
	const auto sorter_id = graph.add_logical_task(
		std::make_shared<m3bp::ValueSortLogicalTask>(comparator));
	const auto receiver_id = graph.add_logical_task(receiver);
	graph
		.add_edge(
			m3bp::LogicalGraph::Port(sender_id, 0),
			m3bp::LogicalGraph::Port(shuffler_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER)
		.add_edge(
			m3bp::LogicalGraph::Port(shuffler_id, 0),
			m3bp::LogicalGraph::Port(sorter_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER)
		.add_edge(
			m3bp::LogicalGraph::Port(sorter_id, 0),
			m3bp::LogicalGraph::Port(receiver_id, 0),
			m3bp::LogicalGraph::PhysicalSuccessor::BARRIER);
	util::execute_logical_graph(graph, concurrency);

	std::vector<PairType> actual;
	for(m3bp::identifier_type p = 0; p < partition_count; ++p){
		auto received = receiver->received_data(p);
		for(const auto &group : received){
			const auto &values = group.second;
			const auto count = values.size();
			actual.emplace_back(group.first, values[0]);
			std::vector<uint8_t> prev_bytes(util::binary_length(values[0]));
			util::write_binary(prev_bytes.data(), values[0]);
			for(m3bp::identifier_type i = 1; i < count; ++i){
				actual.emplace_back(group.first, values[i]);
				std::vector<uint8_t> cur_bytes(util::binary_length(values[i]));
				util::write_binary(cur_bytes.data(), values[i]);
				EXPECT_FALSE(comparator(cur_bytes.data(), prev_bytes.data()))
					<< util::read_binary<ValueType>(cur_bytes.data()).second
					<< " < "
					<< util::read_binary<ValueType>(prev_bytes.data()).second;
				prev_bytes = std::move(cur_bytes);
			}
		}
	}
	std::vector<PairType> expected;
	for(const auto &v : dataset){
		for(const auto &p : v){ expected.emplace_back(p); }
	}
	std::sort(expected.begin(), expected.end());
	std::sort(actual.begin(), actual.end());
	EXPECT_EQ(expected, actual);
}

}

TEST(ValueSortTask, ReduceByKeyZeroBuffers){
	run_value_sort_test<int, int>(
		10, 10, 0, 0,
		[](const void *a, const void *b) -> bool {
			const int x = *reinterpret_cast<const int *>(a);
			const int y = *reinterpret_cast<const int *>(b);
			return x < y;
		});
}

TEST(ValueSortTask, ReduceByKeyEmptyBuffers){
	run_value_sort_test<int, int>(
		10, 10, 10, 0,
		[](const void *a, const void *b) -> bool {
			const int x = *reinterpret_cast<const int *>(a);
			const int y = *reinterpret_cast<const int *>(b);
			return x < y;
		});
}

TEST(ValueSortTask, ReduceByKeySmall){
	run_value_sort_test<int, std::string>(
		4, 20, 10, 100,
		[](const void *a, const void *b) -> bool {
			return strcmp(
				reinterpret_cast<const char *>(a),
				reinterpret_cast<const char *>(b)) < 0;
		});
}

TEST(ValueSortTask, ReduceByKeyLarge){
	run_value_sort_test<std::string, int>(
		24, 500, 100, 1000,
		[](const void *a, const void *b) -> bool {
			const int x = *reinterpret_cast<const int *>(a);
			const int y = *reinterpret_cast<const int *>(b);
			return x < y;
		});
}

