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
#include <gtest/gtest.h>
#include "tasks/physical_task.hpp"
#include "tasks/physical_task_command_base.hpp"
#include "context/execution_context.hpp"
#include "scheduler/locality.hpp"

namespace {

class TestCommand : public m3bp::PhysicalTaskCommandBase {
private:
	int *m_destination;
public:
	TestCommand(int *destination)
		: m_destination(destination)
	{ }
	virtual void prepare(
		m3bp::ExecutionContext & /* context */,
		const m3bp::Locality & /* locality */) override
	{
		*m_destination = 50;
	}
	virtual void run(
		m3bp::ExecutionContext & /* context */,
		const m3bp::Locality & /* locality */) override
	{
		*m_destination = 100;
	}
};

}

TEST(PhysicalTask, Execution){
	m3bp::ExecutionContext context;

	const m3bp::LogicalTaskIdentifier lid(1);
	const m3bp::PhysicalTaskIdentifier pid(2);
	int result = 0;

	auto t = std::make_shared<m3bp::PhysicalTask>(
		lid, pid, std::unique_ptr<TestCommand>(new TestCommand(&result)));
	EXPECT_EQ(lid, t->logical_task_id());
	EXPECT_EQ(pid, t->physical_task_id());

	t->prepare(context, m3bp::Locality());
	EXPECT_EQ(50, result);

	t->run(context, m3bp::Locality());
	EXPECT_EQ(100, result);
}

