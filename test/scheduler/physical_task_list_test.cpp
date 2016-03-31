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
#include "tasks/physical_task.hpp"
#include "tasks/physical_task_command_base.hpp"
#include "scheduler/physical_task_list.hpp"

TEST(PhysicalTaskList, PushPop){
	std::vector<std::shared_ptr<m3bp::PhysicalTask>> tasks = {
		std::make_shared<m3bp::PhysicalTask>(),
		std::make_shared<m3bp::PhysicalTask>(),
		std::make_shared<m3bp::PhysicalTask>(),
		std::make_shared<m3bp::PhysicalTask>()
	};
	m3bp::PhysicalTaskList tlist;
	EXPECT_TRUE(tlist.empty());

	tlist.push_front(tasks[0]);
	EXPECT_FALSE(tlist.empty());
	EXPECT_EQ(tasks[0], tlist.pop_front());
	EXPECT_TRUE(tlist.empty());

	tlist.push_front(tasks[1]);
	EXPECT_FALSE(tlist.empty());
	EXPECT_EQ(tasks[1], tlist.pop_back());
	EXPECT_TRUE(tlist.empty());

	tlist.push_back(tasks[3]);
	EXPECT_FALSE(tlist.empty());
	EXPECT_EQ(tasks[3], tlist.pop_front());
	EXPECT_TRUE(tlist.empty());

	tlist.push_back(tasks[3]);
	EXPECT_FALSE(tlist.empty());
	EXPECT_EQ(tasks[3], tlist.pop_back());
	EXPECT_TRUE(tlist.empty());

	tlist.push_front(tasks[0]);             // [ 0 ]
	EXPECT_FALSE(tlist.empty());
	tlist.push_back (tasks[1]);             // [ 0, 1 ]
	EXPECT_FALSE(tlist.empty());
	tlist.push_front(tasks[2]);             // [ 2, 0, 1 ]
	EXPECT_FALSE(tlist.empty());
	tlist.push_back (tasks[3]);             // [ 2, 0, 1, 3 ]
	EXPECT_FALSE(tlist.empty());
	EXPECT_EQ(tasks[2], tlist.pop_front()); // [ 0, 1, 3 ]
	EXPECT_FALSE(tlist.empty());
	EXPECT_EQ(tasks[0], tlist.pop_front()); // [ 1, 3 ]
	EXPECT_FALSE(tlist.empty());
	tlist.push_back(tasks[2]);              // [ 1, 3, 2 ]
	EXPECT_FALSE(tlist.empty());
	EXPECT_EQ(tasks[1], tlist.pop_front()); // [ 3, 2 ]
	EXPECT_FALSE(tlist.empty());
	EXPECT_EQ(tasks[2], tlist.pop_back());  // [ 3 ]
	EXPECT_FALSE(tlist.empty());
	EXPECT_EQ(tasks[3], tlist.pop_front()); // [ ]
	EXPECT_TRUE(tlist.empty());

	tlist.push_back(std::make_shared<m3bp::PhysicalTask>());
}

TEST(PhysicalTaskList, Synchronization){
	auto task = std::make_shared<m3bp::PhysicalTask>();
	m3bp::PhysicalTaskList tlist;
	volatile bool write_flag = false;
	std::thread sub_thread([&](){
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		write_flag = true;
		tlist.push_back(task);
	});
	std::shared_ptr<m3bp::PhysicalTask> popped;
	while(!popped){
		popped = tlist.pop_front();
	}
	EXPECT_EQ(task, popped);
	EXPECT_TRUE(write_flag);
	sub_thread.join();
}

