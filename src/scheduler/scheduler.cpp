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
#include "scheduler/scheduler.hpp"
#include "scheduler/locality.hpp"
#include "scheduler/locality_option.hpp"
#include "common/random.hpp"
#include "tasks/physical_task.hpp"
#include "tasks/physical_task_command_base.hpp"
#include "logging/general_logger.hpp"
#include "logging/profile_logger.hpp"
#include "logging/profile_event_logger.hpp"

#define M3BP_SCHEDULER_TRACE \
	M3BP_GENERAL_LOG(TRACE) << "[Scheduler] [" << __func__ << "] "

namespace m3bp {

class Scheduler::PhysicalVertex {

public:
	std::atomic<size_type> predecessor_count;
	std::vector<PhysicalTaskIdentifier> successors;
	LocalityOption locality_option;
	PhysicalTaskPtr physical_task;

	PhysicalVertex(LocalityOption option, PhysicalTaskPtr task)
		: predecessor_count(1)
		, successors()
		, locality_option(std::move(option))
		, physical_task(std::move(task))
	{ }

};


Scheduler::Scheduler()
	: m_locality_manager()
	, m_synchronizers(m_locality_manager.max_concurrency())
	, m_stealable_queues(m_locality_manager.node_count())
	, m_unstealable_queues(m_locality_manager.max_concurrency())
	, m_physical_graph_mutex()
	, m_vertices()
	, m_created_task_count(0)
	, m_unfinished_task_count(0)
	, m_cancellation_manager()
{ }

Scheduler::Scheduler(LocalityManager locality_manager)
	: m_locality_manager(std::move(locality_manager))
	, m_synchronizers(m_locality_manager.max_concurrency())
	, m_stealable_queues(m_locality_manager.node_count())
	, m_unstealable_queues(m_locality_manager.max_concurrency())
	, m_physical_graph_mutex()
	, m_vertices()
	, m_created_task_count(0)
	, m_unfinished_task_count(0)
	, m_cancellation_manager()
{ }

Scheduler::~Scheduler() = default;


void Scheduler::notify_all(){
	for(auto &synchronizer : m_synchronizers){
		synchronizer.notify();
	}
}

void Scheduler::decrement_predecessor_count(PhysicalTaskIdentifier task_id){
	std::lock_guard<std::mutex> lock(m_physical_graph_mutex);
	auto it = m_vertices.find(task_id);
	assert(it != m_vertices.end());
	if(--(it->second->predecessor_count) == 0){
		const auto &lo = it->second->locality_option;
		identifier_type w = 0;
		if(!lo.is_stealable()){
			w = lo.recommended_worker();
			m_unstealable_queues[w].push_back(
				std::move(it->second->physical_task));
			m_synchronizers[w].notify();
		}else{
			const auto worker_count = m_locality_manager.max_concurrency();
			if(lo.has_recommendation()){
				w = lo.recommended_worker();
			}else{
				w = uniform_random_integer<identifier_type>(
					0, worker_count - 1);
			}
			const auto node = m_locality_manager.thread_mapping(w);
			m_stealable_queues[node].push_back(
				std::move(it->second->physical_task));
			for(identifier_type i = 0; i < worker_count; ++i){
				const auto t = (w + i) % worker_count;
				if(m_synchronizers[t].notify()){ break; }
			}
		}
	}
}


PhysicalTaskIdentifier Scheduler::create_physical_task(
	LogicalTaskIdentifier logical_task_id,
	std::unique_ptr<PhysicalTaskCommandBase> command,
	const LocalityOption &option)
{
	auto &event_logger = ProfileLogger::thread_local_logger();
	// issue a new physical task ID
	const PhysicalTaskIdentifier physical_task_id(
		static_cast<identifier_type>(m_created_task_count++));
	M3BP_SCHEDULER_TRACE
		<< logical_task_id.identifier() << " "
		<< physical_task_id.identifier() << " ("
		<< option.is_stealable() << " "
		<< (option.has_recommendation()
			? static_cast<int>(option.recommended_worker())
			: -1)
		<< ")";
	event_logger.log_create_physical_task(physical_task_id, logical_task_id);
	// create a task and a vertex
	auto physical_task = std::make_shared<PhysicalTask>(
		logical_task_id, physical_task_id, std::move(command));
	auto vertex = PhysicalVertexPtr(
		new PhysicalVertex(option, std::move(physical_task)));
	// emplace to the set of vertices
	std::lock_guard<std::mutex> lock(m_physical_graph_mutex);
	m_vertices.emplace(physical_task_id, std::move(vertex));
	return physical_task_id;
}

Scheduler &Scheduler::add_dependency(
	PhysicalTaskIdentifier predecessor,
	PhysicalTaskIdentifier successor)
{
	auto &event_logger = ProfileLogger::thread_local_logger();
	M3BP_SCHEDULER_TRACE
		<< predecessor.identifier() << " " << successor.identifier();
	event_logger.log_physical_dependency(predecessor, successor);
	std::lock_guard<std::mutex> lock(m_physical_graph_mutex);
	auto pred_it = m_vertices.find(predecessor);
	if(pred_it == m_vertices.end()){ return *this; }
	auto succ_it = m_vertices.find(successor);
	assert(succ_it != m_vertices.end());
	pred_it->second->successors.emplace_back(std::move(successor));
	++(succ_it->second->predecessor_count);
	return *this;
}

void Scheduler::commit_task(PhysicalTaskIdentifier physical_task_id){
	++m_unfinished_task_count;
	decrement_predecessor_count(physical_task_id);
}


Scheduler::PhysicalTaskPtr
Scheduler::take_unstealable_task(const Locality &locality){
	const auto tid = locality.self_thread_id();
	auto task = m_unstealable_queues[tid].pop_front();
	if(task){ M3BP_SCHEDULER_TRACE << task->physical_task_id().identifier(); }
	return task;
}
Scheduler::PhysicalTaskPtr
Scheduler::take_local_stealable_task(const Locality &locality){
	const auto nid = locality.self_node_id();
	auto task = m_stealable_queues[nid].pop_back();
	if(task){ M3BP_SCHEDULER_TRACE << task->physical_task_id().identifier(); }
	return task;
}
Scheduler::PhysicalTaskPtr
Scheduler::steal_task(const Locality &locality){
	const auto node_count = m_stealable_queues.size();
	const auto nid = locality.self_node_id();
	for(identifier_type i = 0; i < node_count; ++i){
		const auto t = (nid + i) % node_count;
		auto task = m_stealable_queues[t].pop_front();
		if(task){
			M3BP_SCHEDULER_TRACE << task->physical_task_id().identifier();
			return task;
		}
	}
	return PhysicalTaskPtr();
}


Scheduler::PhysicalTaskPtr
Scheduler::take_runnable_task(const Locality &locality){
	const auto tid = locality.self_thread_id();
	PhysicalTaskPtr task;
	// try to take an unstealable task
	task = take_unstealable_task(locality);
	if(task){ return task; }
	// try to take a local stealable task
	task = take_local_stealable_task(locality);
	if(task){ return task; }
	// try to steal an task
	task = steal_task(locality);
	if(task){ return task; }
	// attempt to sleep
	auto &sync = m_synchronizers[tid];
	while(true){
		sync.set_is_sleeping();
		if(task || is_finished() || is_cancelled()){ break; }
		// try to take an unstealable task
		task = take_unstealable_task(locality);
		if(task){ break; }
		// try to steal an task
		task = steal_task(locality);
		if(task){ break; }
		// wait for new task
		sync.wait();
	}
	sync.reset_is_sleeping();
	return task;
}

void Scheduler::notify_task_completion(
	PhysicalTaskIdentifier physical_task_id)
{
	M3BP_SCHEDULER_TRACE << physical_task_id.identifier();
	std::vector<PhysicalTaskIdentifier> successors;
	{
		std::lock_guard<std::mutex> lock(m_physical_graph_mutex);
		auto it = m_vertices.find(physical_task_id);
		assert(it != m_vertices.end());
		successors = std::move(it->second->successors);
		m_vertices.erase(it);
	}
	for(const auto &succ_id : successors){
		decrement_predecessor_count(succ_id);
	}
	const auto remains = --m_unfinished_task_count;
	if(remains == 0){
		notify_all();
	}
}


void Scheduler::notify_exception(std::exception_ptr exception_ptr){
	m_cancellation_manager.notify_exception(std::move(exception_ptr));
	notify_all();
}

void Scheduler::rethrow_exception(){
	m_cancellation_manager.rethrow_exception();
}

bool Scheduler::is_finished() const noexcept {
	return m_unfinished_task_count.load() == 0;
}
bool Scheduler::is_cancelled() const noexcept {
	return m_cancellation_manager.is_cancelled();
}

}

