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
#include <fstream>
#include "context/executor.hpp"
#include "context/execution_context.hpp"
#include "context/worker_thread.hpp"
#include "graph/logical_graph.hpp"
#include "scheduler/scheduler.hpp"
#include "scheduler/locality_manager.hpp"
#include "logging/general_logger.hpp"
#include "logging/profile_logger.hpp"
#include "logging/profile_event_logger.hpp"

namespace m3bp {

Executor::Executor()
	: m_configuration()
	, m_flow_graph()
	, m_thread_observers()
	, m_executor_thread()
	, m_thrown_exception()
	, m_profile_destination()
{ }

void Executor::set_flow_graph(const FlowGraph &graph){
	m_flow_graph = graph;
}

void Executor::set_configuration(const Configuration &config){
	m_configuration = config;
}

void Executor::add_thread_observer(ThreadObserverPtr observer){
	m_thread_observers.emplace_back(std::move(observer));
}


void Executor::execute(){
	if(m_executor_thread.joinable()){
		M3BP_GENERAL_LOG(ERROR)
			<< "Context::execute() is called before the completion of the "
			   "previous execution";
		throw std::runtime_error("Execution thread is already running");
	}
	M3BP_GENERAL_LOG(INFO)
		<< "M3 for Batch Processing version " << M3BP_VERSION;
	m_thrown_exception = nullptr;
	m_executor_thread = std::thread([this](){
		try{
			// prepare contexts
			ExecutionContext ctx(m_configuration, m_flow_graph);
			auto &event_logger = ProfileLogger::thread_local_logger();
			if(ctx.is_profile_enabled()){
				event_logger.enable();
			}else{
				event_logger.disable();
			}
			ctx.logical_graph().create_physical_tasks(ctx);

			// run workers
			const auto worker_count = ctx.configuration().max_concurrency();
			std::vector<WorkerThread> workers(worker_count);
			for(identifier_type i = 0; i < worker_count; ++i){
				workers[i] = WorkerThread(i);
				workers[i].run(ctx, m_thread_observers);
			}
			for(auto &worker : workers){ worker.join(); }

			// cancellation
			auto &graph = ctx.logical_graph();
			auto &scheduler = ctx.scheduler();
			if(scheduler.is_cancelled()){
				for(auto &logical_task : graph.logical_tasks()){
					try{
						logical_task.global_cancel(ctx);
					}catch(...){
						scheduler.notify_exception(std::current_exception());
					}
				}
				scheduler.rethrow_exception();
			}

			if(ctx.is_profile_enabled()){
				ctx.profile_logger().flush_thread_local_log(worker_count);
				const auto profile_destination =
					ctx.configuration().profile_log();
				std::ofstream ofs(profile_destination);
				ctx.profile_logger().dump(ofs);
			}
		}catch(...){
			m_thrown_exception = std::current_exception();
		}
	});
}

void Executor::wait(){
	m_executor_thread.join();
	if(m_thrown_exception){
		std::rethrow_exception(m_thrown_exception);
	}
}

}

