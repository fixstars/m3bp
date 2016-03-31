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
#include "m3bp/context.hpp"
#include "m3bp/configuration.hpp"
#include "m3bp/flow_graph.hpp"
#include "util/workloads/hash_join.hpp"
#include "util/processors/input_generator.hpp"
#include "util/processors/output_receiver.hpp"
#include "util/processors/hash_join_processor.hpp"

TEST(Context, EmptyGraph){
	m3bp::Context ctx;
	ctx.execute();
	ctx.wait();
}

TEST(Context, DoubleExecution){
	m3bp::Context ctx;
	ctx.execute();
	EXPECT_THROW(ctx.execute(), std::runtime_error);
	ctx.wait();
}

TEST(Context, NormalFlow){
	// Hash-Join
	using Workload =
		util::workloads::HashJoinWorkload<int, int, int>;
	using Input0Type = std::pair<int, int>;
	using Input1Type = std::pair<int, int>;
	using ResultType = std::pair<int, std::pair<int, int>>;
	const auto config = m3bp::Configuration()
		.max_concurrency(4);
	Workload workload(100, 10, 100);
	const auto input0 = workload.input0();
	const auto input1 = workload.input1();

	m3bp::FlowGraph fgraph;
	auto output = std::make_shared<std::vector<ResultType>>();
	auto input0_vertex = fgraph.add_vertex(
		"input0", util::processors::TestInputGenerator<Input0Type>(input0));
	auto input1_vertex = fgraph.add_vertex(
		"input1", util::processors::TestInputGenerator<Input1Type>(input1));
	auto join_vertex = fgraph.add_vertex(
		"join", util::processors::TestHashJoinProcessor<int, int, int>());
	auto output_vertex = fgraph.add_vertex(
		"output", util::processors::TestOutputReceiver<ResultType>(output));
	fgraph
		.add_edge(input0_vertex.output_port(0), join_vertex.input_port(0))
		.add_edge(input1_vertex.output_port(0), join_vertex.input_port(1))
		.add_edge(join_vertex.output_port(0), output_vertex.input_port(0));

	m3bp::Context ctx;
	ctx.set_configuration(config);
	ctx.set_flow_graph(fgraph);
	ctx.execute();
	ctx.wait();
	workload.verify(*output);
}

