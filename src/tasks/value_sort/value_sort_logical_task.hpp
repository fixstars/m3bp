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
#ifndef M3BP_TASKS_VALUE_SORT_VALUE_SORT_LOGICAL_TASK_HPP
#define M3BP_TASKS_VALUE_SORT_VALUE_SORT_LOGICAL_TASK_HPP

#include <functional>
#include "tasks/logical_task_base.hpp"
#include "memory/memory_reference.hpp"

namespace m3bp {

class ValueSortLogicalTask : public LogicalTaskBase {

public:
	using ComparatorType = std::function<bool(const void *, const void *)>;

private:
	class ValueSortCommand;

	ComparatorType m_comparator;

public:
	ValueSortLogicalTask();
	ValueSortLogicalTask(ComparatorType comparator);

	virtual void create_physical_tasks(ExecutionContext &context) override;
	virtual void commit_physical_tasks(ExecutionContext &context) override;

protected:
	virtual void receive_fragment(
		ExecutionContext &context,
		identifier_type port,
		identifier_type partition,
		MemoryReference mobj) override;

	void run(
		ExecutionContext &context,
		LockedMemoryReference mobj,
		identifier_type partition);

};

}

#endif

