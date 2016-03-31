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
#include "scheduler/thread_local_scheduler.hpp"

namespace m3bp {

ThreadLocalScheduler::ThreadLocalScheduler()
	: m_stealable_tasks()
	, m_unstealable_tasks()
{ }


void ThreadLocalScheduler::push_stealable_task(PhysicalTaskPtr task){
	m_stealable_tasks.push_front(std::move(task));
}
void ThreadLocalScheduler::push_unstealable_task(PhysicalTaskPtr task){
	m_unstealable_tasks.push_front(std::move(task));
}

ThreadLocalScheduler::PhysicalTaskPtr 
ThreadLocalScheduler::pop_stealable_task(){
	return m_stealable_tasks.pop_front();
}
ThreadLocalScheduler::PhysicalTaskPtr
ThreadLocalScheduler::pop_unstealable_task(){
	return m_unstealable_tasks.pop_front();
}

ThreadLocalScheduler::PhysicalTaskPtr
ThreadLocalScheduler::steal_task(){
	return m_stealable_tasks.pop_back();
}

}

