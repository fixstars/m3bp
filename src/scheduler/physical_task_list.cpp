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
#include <cassert>
#include "tasks/physical_task.hpp"
#include "scheduler/physical_task_list.hpp"
#include "logging/general_logger.hpp"

namespace m3bp {

PhysicalTaskList::PhysicalTaskList()
	: m_mutex()
	, m_deque()
{ }


void PhysicalTaskList::push_front(PhysicalTaskPtr task){
	assert(task);
	std::unique_lock<std::mutex> lock(m_mutex);
	m_deque.emplace_front(std::move(task));
}

void PhysicalTaskList::push_back(PhysicalTaskPtr task){
	assert(task);
	std::unique_lock<std::mutex> lock(m_mutex);
	m_deque.emplace_back(std::move(task));
}


PhysicalTaskList::PhysicalTaskPtr PhysicalTaskList::pop_front(){
	std::unique_lock<std::mutex> lock(m_mutex);
	if(m_deque.empty()){ return PhysicalTaskPtr(); }
	auto task = std::move(m_deque.front());
	m_deque.pop_front();
	return task;
}

PhysicalTaskList::PhysicalTaskPtr PhysicalTaskList::pop_back(){
	std::unique_lock<std::mutex> lock(m_mutex);
	if(m_deque.empty()){ return PhysicalTaskPtr(); }
	auto task = std::move(m_deque.back());
	m_deque.pop_back();
	return task;
}

}

