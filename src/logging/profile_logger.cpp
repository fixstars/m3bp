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
#include <sstream>
#include "logging/profile_logger.hpp"
#include "logging/profile_event_logger.hpp"
#include "m3bp/configuration.hpp"
#include "graph/logical_graph.hpp"

#ifdef M3BP_NO_THREAD_LOCAL
#include "common/thread_specific.hpp"
#endif

namespace m3bp {

void ProfileLogger::set_configuration(const Configuration &config){
	std::ostringstream oss;
	oss << "{";
	oss << "\"max_concurrency\":" << config.max_concurrency() << ",";
	oss << "\"partition_count\":" << config.partition_count() << ",";
	oss << "\"default_output_buffer_size\":"
	    << config.default_output_buffer_size() << ",";
	oss << "\"default_records_per_buffer\":"
	    << config.default_records_per_buffer();
	oss << "}";
	m_configuration_json = oss.str();
}

void ProfileLogger::set_logical_graph(const LogicalGraph &graph){
	{
		std::ostringstream oss;
		oss << "[";
		bool is_first_task = true;
		for(const auto &task : graph.logical_tasks()){
			if(!is_first_task){ oss << ","; }
			is_first_task = false;
			oss << "{";
			oss << "\"task_id\":"   << task.task_id().identifier() << ",";
			oss << "\"task_name\":" << "\"" << task.task_name() << "\"";
			oss << "}";
		}
		oss << "]";
		m_logical_tasks_json = oss.str();
	}
	{
		std::ostringstream oss;
		oss << "[";
		bool is_first_edge = true;
		for(const auto &edge : graph.logical_edges()){
			if(!is_first_edge){ oss << ","; }
			is_first_edge = false;
			const auto p = edge.producer(), c = edge.consumer();
			oss << "{";
			oss << "\"producer\":{";
			oss <<   "\"id\":"   << p.task_id().identifier() << ",";
			oss <<   "\"port\":" << p.port_id()              << "},";
			oss << "\"consumer\":{";
			oss <<   "\"id\":"   << c.task_id().identifier() << ",";
			oss <<   "\"port\":" << c.port_id()              << "}";
			oss << "}";
		}
		oss << "]";
		m_logical_dependencies_json = oss.str();
	}
}


ProfileLogger::ProfileLogger()
	: m_configuration_json("{}")
	, m_logical_tasks_json("[]")
	, m_logical_dependencies_json("[]")
	, m_mutex()
	, m_event_jsons()
{ }

ProfileLogger::ProfileLogger(
	const Configuration &config, const LogicalGraph &graph)
	: m_configuration_json("{}")
	, m_logical_tasks_json("[]")
	, m_logical_dependencies_json("[]")
	, m_mutex()
	, m_event_jsons()
{
	set_configuration(config);
	set_logical_graph(graph);
}


#ifdef M3BP_NO_THREAD_LOCAL
static ThreadSpecific<ProfileEventLogger> g_ts_event_logger;
ProfileEventLogger &ProfileLogger::thread_local_logger(){
	return g_ts_event_logger.get();
}
#else
ProfileEventLogger &ProfileLogger::thread_local_logger(){
	static thread_local ProfileEventLogger s_thread_local_event_logger;
	return s_thread_local_event_logger;
}
#endif

void ProfileLogger::flush_thread_local_log(identifier_type worker_id){
	auto log_json = thread_local_logger().to_json();
	std::lock_guard<std::mutex> lock(m_mutex);
	m_event_jsons.emplace(worker_id, std::move(log_json));
}


void ProfileLogger::dump(std::ostream &os){
	os << "{";
	os << "\"configuration\":" << m_configuration_json << ",";
	os << "\"logical_tasks\":" << m_logical_tasks_json << ",";
	os << "\"logical_dependencies\": "<< m_logical_dependencies_json << ",";
	os << "\"events\":{";
	bool is_first_thread = true;
	for(const auto &p : m_event_jsons){
		if(!is_first_thread){ os << ","; }
		is_first_thread = false;
		os << "\"" << p.first << "\":" << p.second;
	}
	os << "}";
	os << "}" << std::endl;
}

}

