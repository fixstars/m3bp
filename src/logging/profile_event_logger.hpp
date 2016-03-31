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
#ifndef M3BP_LOGGING_PROFILE_EVENT_LOGGER_HPP
#define M3BP_LOGGING_PROFILE_EVENT_LOGGER_HPP

#include <memory>
#include <string>
#include <boost/noncopyable.hpp>
#include "tasks/logical_task_identifier.hpp"
#include "tasks/physical_task_identifier.hpp"
#include "memory/memory_reference.hpp"

namespace m3bp {

class ProfileEventLogger : boost::noncopyable {

private:
	class LogBlock;

	std::unique_ptr<LogBlock> m_current_block;

public:
	ProfileEventLogger();
	~ProfileEventLogger();

	void enable();
	void disable();

	// Physical task creation
	void log_create_physical_task(
		PhysicalTaskIdentifier physical_id, LogicalTaskIdentifier logical_id);
	void log_physical_dependency(
		PhysicalTaskIdentifier producer, PhysicalTaskIdentifier consumer);


	// Physical task execution
	void log_begin_preparation(PhysicalTaskIdentifier task_id);
	void log_end_preparation(PhysicalTaskIdentifier task_id);

	void log_begin_execution(PhysicalTaskIdentifier task_id);
	void log_end_execution(PhysicalTaskIdentifier task_id);


	// Memory management
	void log_allocate_memory(identifier_type mobj_id, size_type size);
	void log_allocate_memory(
		identifier_type mobj_id, size_type size, identifier_type numa_node);
	void log_release_memory(identifier_type mobj_id);

	void log_lock_memory(const MemoryReference &mobj);
	void log_unlock_memory(const MemoryReference &mobj);


	// dump
	std::string to_json() const;

private:
	template <typename Logger, typename... Args>
	void write_binary(Args... args);

	template <typename Logger>
	size_type write_json(std::ostream &os, const void *ptr) const;

};

}

#endif

