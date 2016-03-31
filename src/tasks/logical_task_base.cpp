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
#include "tasks/logical_task_base.hpp"

namespace m3bp {

LogicalTaskBase::LogicalTaskBase()
	: m_task_id()
	, m_task_name("(unnamed)")
	, m_successors()
	, m_entry_task()
	, m_barrier_task()
	, m_terminal_task()
{ }

LogicalTaskBase &LogicalTaskBase::add_successor(
	LogicalTaskPtr dst_task,
	identifier_type dst_port,
	identifier_type src_port)
{
	if(src_port >= m_successors.size()){
		m_successors.resize(src_port + 1);
	}
	m_successors[src_port].emplace_back(std::move(dst_task), dst_port);
	return *this;
}

void LogicalTaskBase::commit_fragment(
	ExecutionContext &context,
	identifier_type port,
	identifier_type partition,
	MemoryReference mobj)
{
	if(port >= m_successors.size()){ return; }
	for(auto &succ : m_successors[port]){
		succ.first->receive_fragment(
			context, succ.second, partition, mobj);
	}
}

}

