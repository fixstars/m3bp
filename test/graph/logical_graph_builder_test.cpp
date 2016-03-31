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
#include "m3bp/configuration.hpp"
#include "m3bp/flow_graph.hpp"
#include "graph/logical_graph_builder.hpp"
#include "util/execution_util.hpp"
#include "util/workloads/broadcast_duplication.hpp"
#include "util/workloads/unite.hpp"
#include "util/workloads/hash_join.hpp"
#include "util/workloads/reduce_by_key.hpp"
#include "util/workloads/sort_join.hpp"
#include "util/processors/input_generator.hpp"
#include "util/processors/output_receiver.hpp"
#include "util/processors/broadcast_duplicator.hpp"
#include "util/processors/unite_processor.hpp"
#include "util/processors/hash_join_processor.hpp"
#include "util/processors/reduce_by_key_processor.hpp"
#include "util/processors/sort_join_processor.hpp"

TEST(LogicalGraphBuilder, BroadcastDuplicator){
	using Workload = util::workloads::BroadcastDuplicationWorkload<int>;
	const auto config = m3bp::Configuration()
		.max_concurrency(4);
	const int task_count = 7;
	Workload workload(task_count, 100);
	const auto input = workload.input();

	m3bp::FlowGraph fgraph;
	auto output = std::make_shared<std::vector<int>>();
	auto input_vertex = fgraph.add_vertex(
		"input", util::processors::TestInputGenerator<int>(input));
	auto dup_vertex = fgraph.add_vertex(
		"dup", util::processors::TestBroadcastDuplicator<int>(task_count));
	auto output_vertex = fgraph.add_vertex(
		"output", util::processors::TestOutputReceiver<int>(output));
	fgraph
		.add_edge(input_vertex.output_port(0), dup_vertex.input_port(0))
		.add_edge(dup_vertex.output_port(0), output_vertex.input_port(0));

	auto lgraph = m3bp::build_logical_graph(fgraph, config);
	util::execute_logical_graph(lgraph, config.max_concurrency());
	workload.verify(*output);
}

TEST(LogicalGraphBuilder, Unite){
	using Workload = util::workloads::UniteWorkload<std::string>;
	const auto config = m3bp::Configuration()
		.max_concurrency(4);
	Workload workload(13, 151);
	const auto input0 = workload.input0(), input1 = workload.input1();

	m3bp::FlowGraph fgraph;
	auto output = std::make_shared<std::vector<std::string>>();
	auto input0_vertex = fgraph.add_vertex(
		"input0", util::processors::TestInputGenerator<std::string>(input0));
	auto input1_vertex = fgraph.add_vertex(
		"input1", util::processors::TestInputGenerator<std::string>(input1));
	auto unite_vertex = fgraph.add_vertex(
		"unite", util::processors::TestUniteProcessor<std::string>());
	auto output_vertex = fgraph.add_vertex(
		"output", util::processors::TestOutputReceiver<std::string>(output));
	fgraph
		.add_edge(input0_vertex.output_port(0), unite_vertex.input_port(0))
		.add_edge(input1_vertex.output_port(0), unite_vertex.input_port(1))
		.add_edge(unite_vertex.output_port(0), output_vertex.input_port(0));

	auto lgraph = m3bp::build_logical_graph(fgraph, config);
	util::execute_logical_graph(lgraph, config.max_concurrency());
	workload.verify(*output);
}

TEST(LogicalGraphBuilder, BuiltinUnite){
	using Workload = util::workloads::UniteWorkload<std::string>;
	const auto config = m3bp::Configuration()
		.max_concurrency(4);
	Workload workload(13, 151);
	const auto input0 = workload.input0(), input1 = workload.input1();

	m3bp::FlowGraph fgraph;
	auto output = std::make_shared<std::vector<std::string>>();
	auto input0_vertex = fgraph.add_vertex(
		"input0", util::processors::TestInputGenerator<std::string>(input0));
	auto input1_vertex = fgraph.add_vertex(
		"input1", util::processors::TestInputGenerator<std::string>(input1));
	auto output_vertex = fgraph.add_vertex(
		"output", util::processors::TestOutputReceiver<std::string>(output));
	fgraph
		.add_edge(input0_vertex.output_port(0), output_vertex.input_port(0))
		.add_edge(input1_vertex.output_port(0), output_vertex.input_port(0));

	auto lgraph = m3bp::build_logical_graph(fgraph, config);
	util::execute_logical_graph(lgraph, config.max_concurrency());
	workload.verify(*output);
}

TEST(LogicalGraphBuilder, HashJoin){
	using Workload =
		util::workloads::HashJoinWorkload<std::string, int, std::string>;
	using Input0Type = std::pair<std::string, int>;
	using Input1Type = std::pair<std::string, std::string>;
	using ResultType = std::pair<std::string, std::pair<int, std::string>>;
	const auto config = m3bp::Configuration()
		.max_concurrency(4);
	Workload workload(100, 11, 200);
	const auto input0 = workload.input0();
	const auto input1 = workload.input1();

	m3bp::FlowGraph fgraph;
	auto output = std::make_shared<std::vector<ResultType>>();
	auto input0_vertex = fgraph.add_vertex(
		"input0", util::processors::TestInputGenerator<Input0Type>(input0));
	auto input1_vertex = fgraph.add_vertex(
		"input1", util::processors::TestInputGenerator<Input1Type>(input1));
	auto join_vertex = fgraph.add_vertex(
		"join",
		util::processors::TestHashJoinProcessor<
			std::string, int, std::string>());
	auto output_vertex = fgraph.add_vertex(
		"output", util::processors::TestOutputReceiver<ResultType>(output));
	fgraph
		.add_edge(input0_vertex.output_port(0), join_vertex.input_port(0))
		.add_edge(input1_vertex.output_port(0), join_vertex.input_port(1))
		.add_edge(join_vertex.output_port(0), output_vertex.input_port(0));

	auto lgraph = m3bp::build_logical_graph(fgraph, config);
	util::execute_logical_graph(lgraph, config.max_concurrency());
	workload.verify(*output);
}

TEST(LogicalGraphBuilder, ReduceByKey){
	using Workload = util::workloads::ReduceByKeyWorkload<std::string, int>;
	using PairType = std::pair<std::string, int>;
	const auto config = m3bp::Configuration()
		.max_concurrency(4);
	Workload workload(200, 100, 100);
	const auto input = workload.input();

	m3bp::FlowGraph fgraph;
	auto output = std::make_shared<std::vector<PairType>>();
	auto input_vertex = fgraph.add_vertex(
		"input", util::processors::TestInputGenerator<PairType>(input));
	auto reduce_vertex = fgraph.add_vertex(
		"reduce",
		util::processors::TestReduceByKeyProcessor<std::string, int>());
	auto output_vertex = fgraph.add_vertex(
		"output", util::processors::TestOutputReceiver<PairType>(output));
	fgraph
		.add_edge(input_vertex.output_port(0), reduce_vertex.input_port(0))
		.add_edge(reduce_vertex.output_port(0), output_vertex.input_port(0));

	auto lgraph = m3bp::build_logical_graph(fgraph, config);
	util::execute_logical_graph(lgraph, config.max_concurrency());
	workload.verify(*output);
}

TEST(LogicalGraphBuilder, UnitedReduceByKey){
	using Workload = util::workloads::ReduceByKeyWorkload<std::string, int>;
	using PairType = std::pair<std::string, int>;
	const auto config = m3bp::Configuration()
		.max_concurrency(4);
	Workload workload(200, 100, 100);
	const auto input = workload.input();
	const auto input_nblocks = input.size();
	const auto input0 = decltype(input)(
		input.begin(), input.begin() + input_nblocks / 2);
	const auto input1 = decltype(input)(
		input.begin() + input_nblocks / 2, input.end());

	m3bp::FlowGraph fgraph;
	auto output = std::make_shared<std::vector<PairType>>();
	auto input0_vertex = fgraph.add_vertex(
		"input0", util::processors::TestInputGenerator<PairType>(input0));
	auto input1_vertex = fgraph.add_vertex(
		"input1", util::processors::TestInputGenerator<PairType>(input1));
	auto reduce_vertex = fgraph.add_vertex(
		"reduce",
		util::processors::TestReduceByKeyProcessor<std::string, int>());
	auto output_vertex = fgraph.add_vertex(
		"output", util::processors::TestOutputReceiver<PairType>(output));
	fgraph
		.add_edge(input0_vertex.output_port(0), reduce_vertex.input_port(0))
		.add_edge(input1_vertex.output_port(0), reduce_vertex.input_port(0))
		.add_edge(reduce_vertex.output_port(0), output_vertex.input_port(0));

	auto lgraph = m3bp::build_logical_graph(fgraph, config);
	util::execute_logical_graph(lgraph, config.max_concurrency());
	workload.verify(*output);
}

TEST(LogicalGraphBuilder, SortJoin){
	using Workload = util::workloads::SortJoinWorkload<int, int, int>;
	using Input0Type = std::pair<int, int>;
	using Input1Type = std::pair<int, int>;
	using ResultType = std::pair<int, std::pair<int, int>>;
	const auto config = m3bp::Configuration()
		.max_concurrency(4);
	Workload workload(10000, 100, 100);
	const auto input0 = workload.input0();
	const auto input1 = workload.input1();

	m3bp::FlowGraph fgraph;
	auto output = std::make_shared<std::vector<ResultType>>();
	auto input0_vertex = fgraph.add_vertex(
		"input0", util::processors::TestInputGenerator<Input0Type>(input0));
	auto input1_vertex = fgraph.add_vertex(
		"input1", util::processors::TestInputGenerator<Input1Type>(input1));
	auto join_vertex = fgraph.add_vertex(
		"join",
		util::processors::TestSortJoinProcessor<int, int, int>());
	auto output_vertex = fgraph.add_vertex(
		"output", util::processors::TestOutputReceiver<ResultType>(output));
	fgraph
		.add_edge(input0_vertex.output_port(0), join_vertex.input_port(0))
		.add_edge(input1_vertex.output_port(0), join_vertex.input_port(1))
		.add_edge(join_vertex.output_port(0), output_vertex.input_port(0));

	auto lgraph = m3bp::build_logical_graph(fgraph, config);
	util::execute_logical_graph(lgraph, config.max_concurrency());
	workload.verify(*output);
}

