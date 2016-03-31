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
#include "m3bp/flow_graph.hpp"
#include "m3bp/processor_base.hpp"
#include "m3bp/context.hpp"
#include "m3bp/exception.hpp"

namespace {

template <m3bp::Movement MOVEMENT>
class InputOnlyProcessor : public m3bp::ProcessorBase {

public:
	InputOnlyProcessor()
		: m3bp::ProcessorBase(
			{ m3bp::InputPort("input").movement(MOVEMENT) },
			{ })
	{ }

	virtual void run(m3bp::Task &) override {
	}

};

template <bool HAS_KEY>
class OutputOnlyProcessor : public m3bp::ProcessorBase {

public:
	OutputOnlyProcessor()
		: m3bp::ProcessorBase(
			{ },
			{ m3bp::OutputPort("output").has_key(HAS_KEY) })
	{ }

	virtual void run(m3bp::Task &) override {
	}

};

template <m3bp::Movement MOVEMENT>
class InOutProcessor : public m3bp::ProcessorBase {

public:
	InOutProcessor()
		: m3bp::ProcessorBase(
			{ m3bp::InputPort("input").movement(MOVEMENT) },
			{ m3bp::OutputPort("output") })
	{ }

	virtual void run(m3bp::Task &) override {
	}

};

template <bool HAS_KEY, m3bp::Movement MOVEMENT>
inline void test_connectivity(){
	m3bp::FlowGraph graph;
	const auto src = graph.add_vertex("src", OutputOnlyProcessor<HAS_KEY>());
	const auto dst = graph.add_vertex("dst", InputOnlyProcessor<MOVEMENT>());
	graph.add_edge(src.output_port(0), dst.input_port(0));
}

}

TEST(FlowGraph, AssertInvalidDescriptor){
	m3bp::FlowGraph graph;
	EXPECT_THROW(
		graph.add_edge(
			m3bp::OutputPortDescriptor(),
			m3bp::InputPortDescriptor()),
		std::invalid_argument);
	const auto src = graph.add_vertex(
		"src", OutputOnlyProcessor<false>());
	const auto dst = graph.add_vertex(
		"dst", InputOnlyProcessor<m3bp::Movement::ONE_TO_ONE>());
	EXPECT_THROW(src.input_port(0),  std::out_of_range);
	EXPECT_THROW(src.output_port(1), std::out_of_range);
	EXPECT_THROW(dst.input_port(1),  std::out_of_range);
	EXPECT_THROW(dst.output_port(0), std::out_of_range);
	EXPECT_THROW(
		graph.add_edge(
			m3bp::OutputPortDescriptor(),
			dst.input_port(0)),
		std::invalid_argument);
	EXPECT_THROW(
		graph.add_edge(
			src.output_port(0),
			m3bp::InputPortDescriptor()),
		std::invalid_argument);
}

TEST(FlowGraph, MismatchedPortSpec){
	EXPECT_NO_THROW(
		(test_connectivity<false, m3bp::Movement::ONE_TO_ONE>()));
	EXPECT_THROW(
		(test_connectivity<false, m3bp::Movement::SCATTER_GATHER>()),
		std::invalid_argument);
	EXPECT_NO_THROW(
		(test_connectivity<false, m3bp::Movement::BROADCAST>()));

	EXPECT_NO_THROW(
		(test_connectivity<true, m3bp::Movement::ONE_TO_ONE>()));
	EXPECT_NO_THROW(
		(test_connectivity<true, m3bp::Movement::SCATTER_GATHER>()));
	EXPECT_NO_THROW(
		(test_connectivity<true, m3bp::Movement::BROADCAST>()));
}

TEST(FlowGraph, CycleDetection){
	m3bp::FlowGraph graph;
	auto v0 = graph.add_vertex(
		"v0", InOutProcessor<m3bp::Movement::ONE_TO_ONE>());
	auto v1 = graph.add_vertex(
		"v1", InOutProcessor<m3bp::Movement::ONE_TO_ONE>());
	auto v2 = graph.add_vertex(
		"v2", InOutProcessor<m3bp::Movement::ONE_TO_ONE>());
	graph.add_edge(v0.output_port(0), v1.input_port(0));
	graph.add_edge(v1.output_port(0), v2.input_port(0));
	graph.add_edge(v2.output_port(0), v0.input_port(0));
	m3bp::Context ctx;
	EXPECT_THROW(ctx.set_flow_graph(graph), m3bp::RoutingError);
}

