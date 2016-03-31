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
#ifndef M3BP_TEST_UTIL_EXECUTION_UTIL_HPP
#define M3BP_TEST_UTIL_EXECUTION_UTIL_HPP

#include <gtest/gtest.h>
#include <vector>
#include <thread>
#include "context/execution_context.hpp"
#include "graph/logical_graph.hpp"
#include "scheduler/scheduler.hpp"
#include "scheduler/locality.hpp"
#include "tasks/physical_task.hpp"
#include "memory/memory_manager.hpp"

namespace util {

static void execute_logical_graph(
	m3bp::LogicalGraph &graph, int concurrency = 4)
{
	m3bp::ExecutionContext context(
		m3bp::Configuration().max_concurrency(concurrency));
	auto &scheduler = context.scheduler();
	auto &memory_manager = context.memory_manager();

	graph.create_physical_tasks(context);
	std::vector<std::thread> threads(concurrency);
	for(int i = 0; i < concurrency; ++i){
		threads[i] = std::thread(
			[i, &scheduler, &context](){
				const m3bp::Locality locality(i, 0);
				while(!scheduler.is_finished()){
					auto task = scheduler.take_runnable_task(locality);
					if(!task){ continue; }
					task->prepare(context, locality);
					task->run(context, locality);
					scheduler.notify_task_completion(task->physical_task_id());
				}
			});
	}
	for(int i = 0; i < concurrency; ++i){
		threads[i].join();
	}
	EXPECT_EQ(0u, memory_manager.total_memory_usage());
	memory_manager.log_memory_leaks();
}

}

#endif

