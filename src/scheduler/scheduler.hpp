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
#ifndef M3BP_SCHEDULER_SCHEDULER_HPP
#define M3BP_SCHEDULER_SCHEDULER_HPP

#include <vector>
#include <unordered_map>
#include <atomic>
#include "common/noncopyable_vector.hpp"
#include "scheduler/physical_task_list.hpp"
#include "scheduler/locality_manager.hpp"
#include "scheduler/cancellation_manager.hpp"
#include "scheduler/scheduler_synchronizer.hpp"
#include "tasks/physical_task_identifier.hpp"
#include "tasks/logical_task_identifier.hpp"

namespace m3bp {

class PhysicalTask;
class PhysicalTaskCommandBase;
class Locality;
class LocalityOption;

class Scheduler {

public:
	using PhysicalTaskPtr = std::shared_ptr<PhysicalTask>;

private:
	class PhysicalVertex;
	using PhysicalVertexPtr = std::unique_ptr<PhysicalVertex>;

	LocalityManager m_locality_manager;

	NoncopyableVector<SchedulerSynchronizer> m_synchronizers;
	NoncopyableVector<PhysicalTaskList> m_stealable_queues;
	NoncopyableVector<PhysicalTaskList> m_unstealable_queues;

	std::mutex m_physical_graph_mutex;
	std::unordered_map<PhysicalTaskIdentifier, PhysicalVertexPtr> m_vertices;

	std::atomic<size_type> m_created_task_count;
	std::atomic<size_type> m_unfinished_task_count;

	CancellationManager m_cancellation_manager;

	void notify_all();
	void decrement_predecessor_count(PhysicalTaskIdentifier task_id);

	PhysicalTaskPtr take_unstealable_task(const Locality &locality);
	PhysicalTaskPtr take_local_stealable_task(const Locality &locality);
	PhysicalTaskPtr steal_task(const Locality &locality);

public:
	Scheduler();
	explicit Scheduler(LocalityManager locality_manager);
	Scheduler(const Scheduler &) = delete;
	~Scheduler();

	Scheduler &operator=(const Scheduler &) = delete;

	PhysicalTaskIdentifier create_physical_task(
		LogicalTaskIdentifier logical_task_id,
		std::unique_ptr<PhysicalTaskCommandBase> command,
		const LocalityOption &option);

	Scheduler &add_dependency(
		PhysicalTaskIdentifier predecessor,
		PhysicalTaskIdentifier successor);

	void commit_task(PhysicalTaskIdentifier physical_task_id);

	PhysicalTaskPtr take_runnable_task(const Locality &locality);
	void notify_task_completion(PhysicalTaskIdentifier physical_task_id);

	void notify_exception(std::exception_ptr exception_ptr);
	void rethrow_exception();

	bool is_finished() const noexcept;
	bool is_cancelled() const noexcept;

};

}

#endif

