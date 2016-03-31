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
#include "m3bp/task.hpp"
#include "api/internal/task_impl.hpp"

namespace m3bp {

Task::Task()
	: m_impl(new internal::TaskImpl())
{ }

Task::Task(internal::TaskImpl &&impl)
	: m_impl(new internal::TaskImpl(std::move(impl)))
{ }

Task::Task(Task &&) = default;

Task::~Task() = default;


Task &Task::operator=(Task &&) = default;


identifier_type Task::logical_task_id() const {
	return m_impl->logical_task_id();
}

identifier_type Task::physical_task_id() const {
	return m_impl->physical_task_id();
}


InputReader Task::input(identifier_type port_id){
	return m_impl->input(port_id);
}

OutputWriter Task::output(identifier_type port_id){
	return m_impl->output(port_id);
}


bool Task::is_cancelled() const {
	return m_impl->is_cancelled();
}

}

