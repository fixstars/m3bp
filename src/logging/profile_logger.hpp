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
#ifndef M3BP_LOGGING_PROFILE_LOGGER_HPP
#define M3BP_LOGGING_PROFILE_LOGGER_HPP

#include <iostream>
#include <map>
#include <string>
#include <mutex>
#include "m3bp/types.hpp"

namespace m3bp {

class Configuration;
class LogicalGraph;
class ProfileEventLogger;

class ProfileLogger {

private:
	std::string m_configuration_json;
	std::string m_logical_tasks_json;
	std::string m_logical_dependencies_json;

	std::mutex m_mutex;
	std::map<identifier_type, std::string> m_event_jsons;

	void set_configuration(const Configuration &config);
	void set_logical_graph(const LogicalGraph &graph);

public:
	ProfileLogger();
	ProfileLogger(
		const Configuration &config,
		const LogicalGraph &graph);

	static ProfileEventLogger &thread_local_logger();
	void flush_thread_local_log(identifier_type worker_id);

	void dump(std::ostream &os);

};

}

#endif

