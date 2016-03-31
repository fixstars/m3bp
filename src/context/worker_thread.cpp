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
#include "m3bp/thread_observer_base.hpp"
#include "context/execution_context.hpp"
#include "context/worker_thread.hpp"
#include "graph/logical_graph.hpp"
#include "scheduler/scheduler.hpp"
#include "scheduler/locality.hpp"
#include "tasks/physical_task.hpp"
#include "system/topology.hpp"
#include "logging/general_logger.hpp"
#include "logging/profile_logger.hpp"
#include "logging/profile_event_logger.hpp"

namespace m3bp {

WorkerThread::WorkerThread()
	: m_thread()
	, m_worker_id(0)
{ }

WorkerThread::WorkerThread(identifier_type worker_id)
	: m_thread()
	, m_worker_id(worker_id)
{ }


void WorkerThread::run(
	ExecutionContext &ctx,
	std::vector<ThreadObserverPtr> observers)
{
	m_thread = std::thread(
		[this, observers, &ctx](){
			// Thread locality binding
			ctx.locality_manager().set_thread_cpubind(m_worker_id);
			const auto worker_id = m_worker_id;
			const auto node_id =
				ctx.locality_manager().thread_mapping(worker_id);
			const Locality self_locality(worker_id, node_id);
			M3BP_GENERAL_LOG(DEBUG)
				<< "Thread #" << worker_id << " is executed "
				<< "(NUMA node #" << node_id << ")";
			// Extract context objects
			auto &graph = ctx.logical_graph();
			auto &scheduler = ctx.scheduler();
			// List of initialized thread observers for cancellation
			std::vector<ThreadObserverPtr> initialized_observers;
			try{
				// Initialize profiling event logger
				auto &event_logger = ProfileLogger::thread_local_logger();
				if(ctx.is_profile_enabled()){
					event_logger.enable();
				}else{
					event_logger.disable();
				}
				// Call ThreadObserverBase::on_initialize()
				initialized_observers.reserve(observers.size());
				for(auto &observer : observers){
					assert(observer);
					observer->on_initialize();
					initialized_observers.emplace_back(std::move(observer));
				}
				// Task loop
				while(!scheduler.is_cancelled() && !scheduler.is_finished()){
					auto task = scheduler.take_runnable_task(self_locality);
					if(!task){ continue; }
					const auto tid = task->physical_task_id();

					event_logger.log_begin_preparation(tid);
					task->prepare(ctx, self_locality);
					event_logger.log_end_preparation(tid);

					event_logger.log_begin_execution(tid);
					task->run(ctx, self_locality);
					event_logger.log_end_execution(tid);

					scheduler.notify_task_completion(task->physical_task_id());
				}
				if(scheduler.is_cancelled()){
					// Call thread_local_cancel for each tasks
					for(auto &logical_task : graph.logical_tasks()){
						logical_task.thread_local_cancel(ctx, self_locality);
					}
				}
				// Call ThreadObserverBase::on_finalize()
				while(!initialized_observers.empty()){
					ThreadObserverPtr moved(
						std::move(initialized_observers.back()));
					initialized_observers.pop_back();
					moved->on_finalize();
				}
				// Dump the profile event log
				if(ctx.is_profile_enabled()){
					ctx.profile_logger().flush_thread_local_log(worker_id);
				}
			}catch(...){
				scheduler.notify_exception(std::current_exception());
				// Call thread_local_cancel for each tasks
				for(auto &logical_task : graph.logical_tasks()){
					try{
						logical_task.thread_local_cancel(ctx, self_locality);
					}catch(...){
						scheduler.notify_exception(std::current_exception());
					}
				}
				// Call ThreadObserverBase::on_finalize()
				while(!initialized_observers.empty()){
					try{
						initialized_observers.back()->on_finalize();
					}catch(...){
						scheduler.notify_exception(std::current_exception());
					}
					initialized_observers.pop_back();
				}
			}
		});
}

void WorkerThread::join(){
	m_thread.join();
}

}

