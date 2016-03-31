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
#ifndef M3BP_TEST_UTIL_PROCESSORS_TEST_PROCESSOR_BASE_HPP
#define M3BP_TEST_UTIL_PROCESSORS_TEST_PROCESSOR_BASE_HPP

#include <gtest/gtest.h>
#include <vector>
#include <set>
#include <thread>
#include <mutex>
#include "m3bp/processor_base.hpp"
#include "m3bp/task.hpp"

namespace util {
namespace processors {

class TestProcessorBase : public m3bp::ProcessorBase {

private:
	std::shared_ptr<std::mutex> m_mutex;

	bool m_global_initialized;
	std::unordered_set<std::thread::id> m_thread_local_initialized;

	std::vector<std::vector<std::vector<uint8_t>>> m_broadcasted;

	std::vector<std::vector<uint8_t>> read_input(
		m3bp::InputReader reader) const
	{
		const auto buffer = reader.raw_buffer();
		const auto num_records = buffer.record_count();
		const auto ptr = static_cast<const uint8_t *>(buffer.key_buffer());
		const auto offsets = buffer.key_offset_table();
		std::vector<std::vector<uint8_t>> result;
		for(m3bp::identifier_type i = 0; i < num_records; ++i){
			result.emplace_back(std::vector<uint8_t>(
				ptr + offsets[i], ptr + offsets[i + 1]));
		}
		return result;
	}

	void validate_broadcast_inputs(m3bp::Task &task) const {
		const auto num_iports = input_ports().size();
		for(m3bp::identifier_type i = 0; i < num_iports; ++i){
			const auto &iport = input_ports()[i];
			if(iport.movement() == m3bp::Movement::BROADCAST){
				const auto input = read_input(task.input(i));
				EXPECT_EQ(m_broadcasted[i], input);
			}
		}
	}

public:
	TestProcessorBase(
		std::vector<m3bp::InputPort> iports,
		std::vector<m3bp::OutputPort> oports)
		: m3bp::ProcessorBase(iports, oports)
		, m_mutex(new std::mutex())
		, m_global_initialized(false)
		, m_thread_local_initialized()
		, m_broadcasted(iports.size())
	{ }

	virtual ~TestProcessorBase(){
		EXPECT_FALSE(m_global_initialized);
		EXPECT_TRUE(m_thread_local_initialized.empty());
	}


	virtual void global_initialize(m3bp::Task &task) override {
		EXPECT_FALSE(m_global_initialized);
		EXPECT_TRUE(m_thread_local_initialized.empty());

		const auto num_iports = input_ports().size();
		for(m3bp::identifier_type i = 0; i < num_iports; ++i){
			const auto &iport = input_ports()[i];
			if(iport.movement() == m3bp::Movement::BROADCAST){
				m_broadcasted[i] = read_input(task.input(i));
			}
		}
		m_global_initialized = true;
	}

	virtual void global_finalize(m3bp::Task &task) override {
		EXPECT_TRUE(m_global_initialized);
		EXPECT_TRUE(m_thread_local_initialized.empty());
		m_global_initialized = false;
		if(!task.is_cancelled()){ validate_broadcast_inputs(task); }
	}


	virtual void thread_local_initialize(m3bp::Task &task) override {
		EXPECT_TRUE(m_global_initialized);
		{
			std::lock_guard<std::mutex> lock(*m_mutex);
			const auto it =
				m_thread_local_initialized.find(std::this_thread::get_id());
			EXPECT_EQ(m_thread_local_initialized.end(), it);
			m_thread_local_initialized.insert(std::this_thread::get_id());
		}
		validate_broadcast_inputs(task);
	}

	virtual void thread_local_finalize(m3bp::Task &task) override {
		EXPECT_TRUE(m_global_initialized);
		{
			std::lock_guard<std::mutex> lock(*m_mutex);
			const auto it =
				m_thread_local_initialized.find(std::this_thread::get_id());
			EXPECT_NE(m_thread_local_initialized.end(), it);
			m_thread_local_initialized.erase(it);
		}
		if(!task.is_cancelled()){ validate_broadcast_inputs(task); }
	}

	virtual void run(m3bp::Task &task) override {
		EXPECT_TRUE(m_global_initialized);
		{
			std::lock_guard<std::mutex> lock(*m_mutex);
			const auto it =
				m_thread_local_initialized.find(std::this_thread::get_id());
			EXPECT_NE(m_thread_local_initialized.end(), it);
		}
		validate_broadcast_inputs(task);
	}

};

}
}

#endif

