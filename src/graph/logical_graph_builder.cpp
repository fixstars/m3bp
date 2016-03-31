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
#include <map>
#include <cassert>
#include "m3bp/configuration.hpp"
#include "graph/logical_graph_builder.hpp"
#include "api/internal/flow_graph_impl.hpp"
#include "tasks/gather/gather_logical_task.hpp"
#include "tasks/shuffle/shuffle_logical_task.hpp"
#include "tasks/value_sort/value_sort_logical_task.hpp"
#include "tasks/process/input_process_logical_task.hpp"
#include "tasks/process/one_to_one_process_logical_task.hpp"
#include "tasks/process/scatter_gather_process_logical_task.hpp"
#include "logging/general_logger.hpp"

namespace m3bp {
namespace {

class LogicalGraphBuilder {

public:
	using PortKey = std::pair<identifier_type, identifier_type>;
	using PortSet = std::vector<PortKey>;
	using PortSetToTaskMap = std::map<PortSet, LogicalTaskIdentifier>;

private:
	FlowGraph m_flow_graph;
	Configuration m_configuration;

	LogicalGraph m_logical_graph;
	PortSetToTaskMap m_shuffle_nodes;
	PortSetToTaskMap m_gather_nodes;

	std::unique_ptr<LogicalTaskBase>
	create_process_task(const internal::ProcessorWrapper &pw) const {
		const auto &config = m_configuration;
		bool is_one_to_one = false, is_scatter_gather = false;
		for(const auto &iport : pw->input_ports()){
			if(iport.movement() == Movement::ONE_TO_ONE){
				is_one_to_one = true;
			}else if(iport.movement() == Movement::SCATTER_GATHER){
				is_scatter_gather = true;
			}
		}
		assert(!(is_one_to_one && is_scatter_gather));
		if(is_one_to_one){
			return std::unique_ptr<LogicalTaskBase>(
				new OneToOneProcessLogicalTask(pw, config.max_concurrency()));
		}else if(is_scatter_gather){
			return std::unique_ptr<LogicalTaskBase>(
				new ScatterGatherProcessLogicalTask(
					pw, config.max_concurrency(), config.partition_count()));
		}else{
			return std::unique_ptr<LogicalTaskBase>(
				new InputProcessLogicalTask(pw, config.max_concurrency()));
		}
	}

	void create_processor_nodes(){
		const auto &fg_impl = internal::FlowGraphImpl::get_impl(m_flow_graph);
		const auto vertices = fg_impl.vertices();
		for(const auto &v : vertices){
			auto task = create_process_task(v.processor());
			task->task_name(v.name());
			m_logical_graph.add_logical_task(std::move(task));
		}
	}

	template <typename Iterator>
	PortSet normalize_port_set(Iterator begin, Iterator end){
		PortSet pset;
		for(Iterator it = begin; it != end; ++it){
			pset.emplace_back(it->vertex_id(), it->port_id());
		}
		std::sort(pset.begin(), pset.end());
		return pset;
	}

	std::string concat_port_names(const PortSet &ps) const {
		const auto &fg_impl = internal::FlowGraphImpl::get_impl(m_flow_graph);
		const auto vertices = fg_impl.vertices();
		std::ostringstream oss;
		bool is_first = true;
		oss << "(";
		for(const auto &p : ps){
			if(!is_first){ oss << "+"; }
			is_first = false;
			const auto &src_vertex = vertices[p.first];
			const auto &oports = src_vertex.processor()->output_ports();
			oss << src_vertex.name() << "." << oports[p.second].name();
		}
		oss << ")";
		return oss.str();
	}

	LogicalTaskIdentifier create_shuffle_node(const PortSet &ps){
		const auto it = m_shuffle_nodes.find(ps);
		if(it != m_shuffle_nodes.end()){ return it->second; }
		// Create a shuffle node
		auto task = std::unique_ptr<LogicalTaskBase>(
			new ShuffleLogicalTask(m_configuration.partition_count()));
		task->task_name(concat_port_names(ps) + ".shuffle");
		const auto shuffle_lid =
			m_logical_graph.add_logical_task(std::move(task));
		// Connect from sources of the created shuffle node
		for(const auto &p : ps){
			const LogicalTaskIdentifier src_lid(p.first);
			m_logical_graph.add_edge(
				LogicalGraph::Port(src_lid, p.second),
				LogicalGraph::Port(shuffle_lid, 0),
				LogicalGraph::PhysicalSuccessor::BARRIER);
		}
		m_shuffle_nodes.emplace(ps, shuffle_lid);
		return shuffle_lid;
	}

	LogicalTaskIdentifier create_gather_node(const PortSet &ps){
		const auto it = m_gather_nodes.find(ps);
		if(it != m_gather_nodes.end()){ return it->second; }
		// Create a gather node
		auto task = std::unique_ptr<LogicalTaskBase>(new GatherLogicalTask());
		task->task_name(concat_port_names(ps) + ".gather");
		const auto gather_lid =
			m_logical_graph.add_logical_task(std::move(task));
		// Connect from sources of the created gather node
		for(const auto &p : ps){
			const LogicalTaskIdentifier src_lid(p.first);
			m_logical_graph.add_edge(
				LogicalGraph::Port(src_lid, p.second),
				LogicalGraph::Port(gather_lid, 0),
				LogicalGraph::PhysicalSuccessor::ENTRY);
		}
		m_gather_nodes.emplace(ps, gather_lid);
		return gather_lid;
	}

	void create_intermediate_nodes(){
		const auto &fg_impl = internal::FlowGraphImpl::get_impl(m_flow_graph);
		const auto vertices = fg_impl.vertices();
		for(identifier_type i = 0; i < vertices.size(); ++i){
			const auto &v = vertices[i];
			const auto sources = v.sources();
			const auto &iports = v.processor()->input_ports();
			for(identifier_type j = 0; j < iports.size(); ++j){
				if(iports[j].movement() == Movement::SCATTER_GATHER){
					create_shuffle_node(normalize_port_set(
						sources[j].begin(), sources[j].end()));
				}else if(iports[j].movement() == Movement::BROADCAST){
					create_gather_node(normalize_port_set(
						sources[j].begin(), sources[j].end()));
				}
			}
		}
	}

	void create_edges(){
		const auto &fg_impl = internal::FlowGraphImpl::get_impl(m_flow_graph);
		const auto vertices = fg_impl.vertices();
		for(identifier_type i = 0; i < vertices.size(); ++i){
			const auto &v = vertices[i];
			const auto &iports = v.processor()->input_ports();
			const auto sources = v.sources();
			const auto iport_count = iports.size();
			for(identifier_type j = 0; j < iport_count; ++j){
				const auto ps = normalize_port_set(
					sources[j].begin(), sources[j].end());
				auto comparator = iports[j].value_comparator();
				if(iports[j].movement() == Movement::ONE_TO_ONE){
					// one-to-one
					for(const auto &src : sources[j]){
						m_logical_graph.add_edge(
							LogicalGraph::Port(
								LogicalTaskIdentifier(src.vertex_id()),
								src.port_id()),
							LogicalGraph::Port(LogicalTaskIdentifier(i), j),
							LogicalGraph::PhysicalSuccessor::BARRIER);
					}
				}else if(iports[j].movement() == Movement::BROADCAST){
					// broadcast
					m_logical_graph.add_edge(
						LogicalGraph::Port(m_gather_nodes.at(ps), 0),
						LogicalGraph::Port(LogicalTaskIdentifier(i), j),
						LogicalGraph::PhysicalSuccessor::ENTRY);
				}else if(comparator){
					// scatter-gather with in-group sorting
					auto sort_task = std::unique_ptr<LogicalTaskBase>(
						new ValueSortLogicalTask(std::move(comparator)));
					sort_task->task_name(
						v.name()         + "." +
						iports[j].name() + "." +
						"value_sort");
					const auto sort_id =
						m_logical_graph.add_logical_task(std::move(sort_task));
					m_logical_graph.add_edge(
						LogicalGraph::Port(m_shuffle_nodes.at(ps), 0),
						LogicalGraph::Port(sort_id, 0),
						LogicalGraph::PhysicalSuccessor::TERMINAL);
					m_logical_graph.add_edge(
						LogicalGraph::Port(sort_id, 0),
						LogicalGraph::Port(LogicalTaskIdentifier(i), j),
						LogicalGraph::PhysicalSuccessor::BARRIER);
				}else{
					// scatter-gather
					m_logical_graph.add_edge(
						LogicalGraph::Port(m_shuffle_nodes.at(ps), 0),
						LogicalGraph::Port(LogicalTaskIdentifier(i), j),
						LogicalGraph::PhysicalSuccessor::BARRIER);
				}
			}
		}
	}

public:
	LogicalGraphBuilder(FlowGraph flow_graph, const Configuration &config)
		: m_flow_graph(std::move(flow_graph))
		, m_configuration(config)
		, m_logical_graph()
		, m_shuffle_nodes()
		, m_gather_nodes()
	{ }

	LogicalGraph build(){
		m_logical_graph = LogicalGraph();
		m_shuffle_nodes.clear();
		m_gather_nodes.clear();

		create_processor_nodes();
		create_intermediate_nodes();
		create_edges();

		auto logical_graph = std::move(m_logical_graph);
		return logical_graph;
	}

};

}


LogicalGraph build_logical_graph(
	FlowGraph flow_graph, const Configuration &config)
{
	LogicalGraphBuilder builder(std::move(flow_graph), config);
	return builder.build();
}

}

