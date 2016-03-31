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
#include <thread>
#include <gtest/gtest.h>
#include "context/execution_context.hpp"
#include "scheduler/locality.hpp"
#include "scheduler/locality_option.hpp"
#include "tasks/physical_task.hpp"
#include "tasks/physical_task_command_base.hpp"

namespace {

class TestCommand : public m3bp::PhysicalTaskCommandBase {
private:
	int *m_destination;
	int m_value;
public:
	TestCommand(int *destination, int value)
		: m_destination(destination)
		, m_value(value)
	{ }
	virtual void run(
		m3bp::ExecutionContext & /* context */,
		const m3bp::Locality & /* locality */) override
	{
		*m_destination = m_value;
	}
};

struct EntryTerminalPair {
	m3bp::PhysicalTaskIdentifier entry_id;
	m3bp::PhysicalTaskIdentifier terminal_id;
};

class TaskGeneratorCommand : public m3bp::PhysicalTaskCommandBase {
private:
	EntryTerminalPair *m_et_pair;
	int *m_destination;
	int m_value;
public:
	TaskGeneratorCommand(
		EntryTerminalPair *entry_terminal_pair, int *destination, int value)
		: m_et_pair(entry_terminal_pair)
		, m_destination(destination)
		, m_value(value)
	{ }
	virtual void run(
		m3bp::ExecutionContext &context,
		const m3bp::Locality & /* locality */) override
	{
		auto &scheduler = context.scheduler();
		const m3bp::LogicalTaskIdentifier lid(1);
		const auto dst = m_destination;
		*dst = m_value;
		auto t0 = scheduler.create_physical_task(
			lid, std::unique_ptr<TestCommand>(new TestCommand(dst, 20)),
			m3bp::LocalityOption());
		auto t1 = scheduler.create_physical_task(
			lid, std::unique_ptr<TestCommand>(new TestCommand(dst, 30)),
			m3bp::LocalityOption());
		scheduler.add_dependency(m_et_pair->entry_id, t0);
		scheduler.add_dependency(m_et_pair->entry_id, t1);
		scheduler.add_dependency(t0, m_et_pair->terminal_id);
		scheduler.add_dependency(t1, m_et_pair->terminal_id);
		scheduler.commit_task(t0);
		scheduler.commit_task(t1);
	}
};

}

TEST(Scheduler, SingleTask){
	m3bp::ExecutionContext context(
		m3bp::Configuration().max_concurrency(1));
	auto &scheduler = context.scheduler();
	const m3bp::LogicalTaskIdentifier lid(1);
	const m3bp::Locality locality(0, 0);

	int result = 0;
	auto command = std::unique_ptr<TestCommand>(new TestCommand(&result, 10));
	auto task_id = scheduler.create_physical_task(
		lid, std::move(command), m3bp::LocalityOption());
	scheduler.commit_task(task_id);

	auto taken = scheduler.take_runnable_task(locality);
	EXPECT_EQ(0, result);
	taken->run(context, locality);
	EXPECT_EQ(10, result);
	scheduler.notify_task_completion(taken->physical_task_id());
}

TEST(Scheduler, Dependency){
	m3bp::ExecutionContext context(
		m3bp::Configuration().max_concurrency(1));
	auto &scheduler = context.scheduler();
	const m3bp::LogicalTaskIdentifier lid(1);
	const m3bp::Locality locality(0, 0);
	EXPECT_TRUE(scheduler.is_finished());

	int result = 0;
	auto t0 = scheduler.create_physical_task(
		lid, std::unique_ptr<TestCommand>(new TestCommand(&result, 10)),
		m3bp::LocalityOption());
	auto t1 = scheduler.create_physical_task(
		lid, std::unique_ptr<TestCommand>(new TestCommand(&result, 20)),
		m3bp::LocalityOption());
	auto t2 = scheduler.create_physical_task(
		lid, std::unique_ptr<TestCommand>(new TestCommand(&result, 30)),
		m3bp::LocalityOption());
	scheduler.add_dependency(t0, t2);
	scheduler.add_dependency(t1, t2);
	scheduler.commit_task(t0);
	scheduler.commit_task(t1);
	scheduler.commit_task(t2);
	EXPECT_FALSE(scheduler.is_finished());

	auto taken0 = scheduler.take_runnable_task(locality);
	taken0->run(context, locality);
	EXPECT_TRUE(result == 10 || result == 20);
	const int first_result = result;

	auto taken1 = scheduler.take_runnable_task(locality);
	taken1->run(context, locality);
	EXPECT_TRUE((result == 10 || result == 20) && result != first_result);

	scheduler.notify_task_completion(taken0->physical_task_id());
	scheduler.notify_task_completion(taken1->physical_task_id());

	auto taken2 = scheduler.take_runnable_task(locality);
	taken2->run(context, locality);
	EXPECT_EQ(30, result);
	scheduler.notify_task_completion(taken2->physical_task_id());
	EXPECT_TRUE(scheduler.is_finished());
}

TEST(Scheduler, CreateInTask){
	m3bp::ExecutionContext context(
		m3bp::Configuration().max_concurrency(1));
	auto &scheduler = context.scheduler();
	const m3bp::LogicalTaskIdentifier lid(1);
	const m3bp::Locality locality(0, 0);

	int result = 0;
	EntryTerminalPair et_pair;
	et_pair.entry_id = scheduler.create_physical_task(
		lid, std::unique_ptr<TaskGeneratorCommand>(
			new TaskGeneratorCommand(&et_pair, &result, 10)),
		m3bp::LocalityOption());
	et_pair.terminal_id = scheduler.create_physical_task(
		lid, std::unique_ptr<TestCommand>(new TestCommand(&result, 40)),
		m3bp::LocalityOption());
	scheduler.add_dependency(et_pair.entry_id, et_pair.terminal_id);
	scheduler.commit_task(et_pair.terminal_id);
	scheduler.commit_task(et_pair.entry_id);

	auto taken0 = scheduler.take_runnable_task(locality);
	taken0->run(context, locality);
	EXPECT_EQ(10, result);
	scheduler.notify_task_completion(taken0->physical_task_id());

	auto taken1 = scheduler.take_runnable_task(locality);
	auto taken2 = scheduler.take_runnable_task(locality);
	taken1->run(context, locality);
	const int t1_result = result;
	taken2->run(context, locality);
	const int t2_result = result;
	EXPECT_TRUE(
		(t1_result == 20 && t2_result == 30) ||
		(t1_result == 30 && t2_result == 20));
	scheduler.notify_task_completion(taken1->physical_task_id());
	scheduler.notify_task_completion(taken2->physical_task_id());

	auto taken3 = scheduler.take_runnable_task(locality);
	taken3->run(context, locality);
	EXPECT_EQ(40, result);
	scheduler.notify_task_completion(taken3->physical_task_id());
}

TEST(Scheduler, MultiThreadedDependency){
	m3bp::ExecutionContext context(
		m3bp::Configuration().max_concurrency(2));
	auto &scheduler = context.scheduler();
	const m3bp::LogicalTaskIdentifier lid(1);
	const m3bp::Locality locality(0, 0);

	int result = 0;
	auto t0 = scheduler.create_physical_task(
		lid, std::unique_ptr<TestCommand>(new TestCommand(&result, 10)),
		m3bp::LocalityOption());
	auto t1 = scheduler.create_physical_task(
		lid, std::unique_ptr<TestCommand>(new TestCommand(&result, 20)),
		m3bp::LocalityOption());
	scheduler.add_dependency(t0, t1);
	scheduler.commit_task(t0);

	auto taken0 = scheduler.take_runnable_task(locality);
	auto thread = std::thread([&](){
		auto taken1 = scheduler.take_runnable_task(m3bp::Locality());
		taken1->run(context, m3bp::Locality(0, 1));
		EXPECT_EQ(20, result);
		scheduler.notify_task_completion(taken1->physical_task_id());
	});
	taken0->run(context, locality);
	EXPECT_EQ(10, result);
	scheduler.commit_task(t1);
	scheduler.notify_task_completion(taken0->physical_task_id());
	thread.join();
	EXPECT_EQ(20, result);
	EXPECT_TRUE(scheduler.is_finished());
}

TEST(Scheduler, IsFinished){
	m3bp::ExecutionContext context(
		m3bp::Configuration().max_concurrency(1));
	auto &scheduler = context.scheduler();
	const m3bp::LogicalTaskIdentifier lid(1);
	const m3bp::Locality locality(0, 0);

	int result = 0;
	auto t0 = scheduler.create_physical_task(
		lid, std::unique_ptr<TestCommand>(new TestCommand(&result, 10)),
		m3bp::LocalityOption());
	auto t1 = scheduler.create_physical_task(
		lid, std::unique_ptr<TestCommand>(new TestCommand(&result, 20)),
		m3bp::LocalityOption());
	scheduler.add_dependency(t0, t1);
	scheduler.commit_task(t0);
	scheduler.commit_task(t1);
	EXPECT_FALSE(scheduler.is_finished());

	auto taken0 = scheduler.take_runnable_task(locality);
	scheduler.notify_task_completion(taken0->physical_task_id());
	EXPECT_FALSE(scheduler.is_finished());
	auto taken1 = scheduler.take_runnable_task(locality);
	scheduler.notify_task_completion(taken1->physical_task_id());
	EXPECT_TRUE(scheduler.is_finished());
	auto taken2 = scheduler.take_runnable_task(locality);
	EXPECT_EQ(nullptr, taken2.get());
}
