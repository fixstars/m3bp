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
#include <cstring>
#include <gtest/gtest.h>
#include "m3bp/output_writer.hpp"
#include "common/make_unique.hpp"
#include "api/internal/output_writer_impl.hpp"
#include "tasks/logical_task_base.hpp"
#include "util/binary_util.hpp"
#include "util/generator_util.hpp"
#include "util/receiver_task.hpp"

namespace {

class DummyLogicalTask : public m3bp::LogicalTaskBase {
public:
	DummyLogicalTask()
		: m3bp::LogicalTaskBase()
	{ }
	virtual void create_physical_tasks(m3bp::ExecutionContext &) override { }
	virtual void commit_physical_tasks(m3bp::ExecutionContext &) override { }
};

}

TEST(OutputWriter, AllocateDefault){
	const m3bp::size_type default_buffer_size = 482;
	const m3bp::size_type default_records_per_buffer = 51;
	m3bp::ExecutionContext context;
	{
		auto logical_task = std::make_shared<DummyLogicalTask>();
		m3bp::internal::OutputWriterImpl writer_impl;
		writer_impl
			.context                   (&context)
			.processor_task            (logical_task.get())
			.output_port               (0)
			.has_keys                  (false)
			.default_buffer_size       (default_buffer_size)
			.default_records_per_buffer(default_records_per_buffer);
		auto writer =
			m3bp::internal::OutputWriterImpl::wrap_impl(
				std::move(writer_impl));

		auto buffer = writer.allocate_buffer();
		EXPECT_NE(nullptr,                    buffer.data_buffer());
		EXPECT_LE(default_buffer_size,        buffer.data_buffer_size());
		EXPECT_LE(default_records_per_buffer, buffer.max_record_count());
		EXPECT_EQ(nullptr,                    buffer.key_length_table());
	}
}

TEST(OutputWriter, AllocateWithSize){
	m3bp::ExecutionContext context;
	{
		auto logical_task = std::make_shared<DummyLogicalTask>();
		m3bp::internal::OutputWriterImpl writer_impl;
		writer_impl
			.context       (&context)
			.processor_task(logical_task.get())
			.output_port   (0)
			.has_keys      (true);
		auto writer =
			m3bp::internal::OutputWriterImpl::wrap_impl(
				std::move(writer_impl));

		auto default_buffer = writer.allocate_buffer();
		EXPECT_NE(nullptr, default_buffer.key_length_table());

		const auto req_data_buffer_size =
			default_buffer.data_buffer_size() * 2;
		const auto req_max_record_count =
			default_buffer.max_record_count() * 3;
		auto req_buffer = writer.allocate_buffer(
			req_data_buffer_size, req_max_record_count);
		EXPECT_NE(nullptr,              req_buffer.data_buffer());
		EXPECT_LE(req_data_buffer_size, req_buffer.data_buffer_size());
		EXPECT_LE(req_max_record_count, req_buffer.max_record_count());
		EXPECT_NE(nullptr,              req_buffer.key_length_table());
	}
}

TEST(OutputWriter, FlushBuffer){
	using PairType = std::pair<int, std::string>;
	m3bp::ExecutionContext context;
	{
		auto logical_task = std::make_shared<DummyLogicalTask>();
		auto receiver_task = std::make_shared<util::ReceiverTask<PairType>>(1);
		logical_task->add_successor(receiver_task, 0, 0);
		m3bp::internal::OutputWriterImpl writer_impl;
		writer_impl
			.context       (&context)
			.processor_task(logical_task.get())
			.output_port   (0)
			.has_keys      (true);
		auto writer =
			m3bp::internal::OutputWriterImpl::wrap_impl(std::move(writer_impl));

		const int NUM_RECORDS = 101;
		std::vector<std::pair<int, std::string>> dataset(NUM_RECORDS);
		m3bp::size_type total_size = 0;
		for(auto &x : dataset){
			x = std::make_pair(
				util::generate_random<int>(),
				util::generate_random<std::string>());
			total_size += util::binary_length(x);
		}

		auto buffer = writer.allocate_buffer(total_size, NUM_RECORDS);
		uint8_t *ptr_u8  = reinterpret_cast<uint8_t *>(buffer.data_buffer());
		auto offsets     = buffer.offset_table();
		auto key_lengths = buffer.key_length_table();
		m3bp::size_type written_bytes = 0;
		offsets[0] = 0;
		for(int i = 0; i < NUM_RECORDS; ++i){
			util::write_binary(ptr_u8 + written_bytes, dataset[i]);
			written_bytes += util::binary_length(dataset[i]);
			offsets[i + 1] = offsets[i] + util::binary_length(dataset[i]);
			key_lengths[i] = util::binary_length(dataset[i].first);
		}
		writer.flush_buffer(std::move(buffer), NUM_RECORDS);
		EXPECT_EQ(nullptr, buffer.data_buffer());

		const auto actual = receiver_task->received_data(0);
		EXPECT_EQ(dataset.size(), actual.size());
		EXPECT_EQ(dataset.back(), actual.back());
		EXPECT_EQ(dataset, actual);
	}
}

