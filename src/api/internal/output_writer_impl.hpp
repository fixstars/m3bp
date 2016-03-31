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
#ifndef M3BP_API_INTERNAL_OUTPUT_WRITER_IMPL_HPP
#define M3BP_API_INTERNAL_OUTPUT_WRITER_IMPL_HPP

#include <iostream>
#include <mutex>
#include "m3bp/output_writer.hpp"
#include "api/internal/output_buffer_impl.hpp"
#include "memory/memory_reference.hpp"
#include "memory/serialized_buffer.hpp"
#include "tasks/logical_task_base.hpp"
#include "context/execution_context.hpp"
#include "scheduler/locality.hpp"

namespace m3bp {

class Scheduler;

namespace internal {

class OutputWriterImpl {

private:
	ExecutionContext *m_context;
	Locality m_current_locality;

	LogicalTaskBase *m_logical_task;
	identifier_type m_output_port;
	bool m_has_keys;

	size_type m_default_buffer_size;
	size_type m_default_records_per_buffer;

public:
	OutputWriterImpl()
		: m_context(nullptr)
		, m_current_locality()
		, m_logical_task(nullptr)
		, m_output_port(0)
		, m_has_keys(false)
		, m_default_buffer_size(4 << 20)        // 4MB
		, m_default_records_per_buffer(1 << 20) // 1M records
	{ }

	OutputBuffer allocate_buffer(){
		return allocate_buffer(
			m_default_buffer_size, m_default_records_per_buffer);
	}

	OutputBuffer allocate_buffer(
		size_type min_data_size, size_type min_record_count)
	{
		if(!m_context){
			throw std::runtime_error(
				"This OutputWriter is not corresponding to any tasks");
		}
		auto &memory_manager = m_context->memory_manager();
		SerializedBuffer sb;
		if(m_has_keys){
			sb = SerializedBuffer::allocate_key_value_buffer(
				memory_manager,
				std::max(min_record_count, m_default_records_per_buffer),
				std::max(min_data_size,    m_default_buffer_size),
				m_current_locality.self_node_id());
		}else{
			sb = SerializedBuffer::allocate_value_only_buffer(
				memory_manager,
				std::max(min_record_count, m_default_records_per_buffer),
				std::max(min_data_size,    m_default_buffer_size),
				m_current_locality.self_node_id());
		}
		OutputBufferImpl buffer_impl;
		buffer_impl.bind_fragment(std::move(sb));
		return OutputBufferImpl::wrap_impl(std::move(buffer_impl));
	}

	void flush_buffer(OutputBuffer buffer, size_type record_count){
		if(!m_context){
			throw std::runtime_error(
				"This OutputWriter is not corresponding to any tasks");
		}
		if(record_count > 0){
			auto sb = OutputBufferImpl::get_impl(buffer).unbind_fragment();
			sb.record_count(record_count);
			m_logical_task->commit_fragment(
				*m_context,
				m_output_port,
				0,
				MemoryReference(sb.raw_reference()));
		}
	}

	OutputWriterImpl &context(ExecutionContext *context){
		m_context = context;
		return *this;
	}
	OutputWriterImpl &current_locality(const Locality &locality){
		m_current_locality = locality;
		return *this;
	}
	OutputWriterImpl &processor_task(LogicalTaskBase *task){
		m_logical_task = task;
		return *this;
	}
	OutputWriterImpl &output_port(identifier_type port){
		m_output_port = port;
		return *this;
	}
	OutputWriterImpl &has_keys(bool flag){
		m_has_keys = flag;
		return *this;
	}

	OutputWriterImpl &default_buffer_size(size_type size){
		m_default_buffer_size = size;
		return *this;
	}
	OutputWriterImpl &default_records_per_buffer(size_type count){
		m_default_records_per_buffer = count;
		return *this;
	}


	static OutputWriterImpl &get_impl(OutputWriter &writer){
		return *writer.m_impl;
	}

	static OutputWriter wrap_impl(OutputWriterImpl impl){
		return OutputWriter(std::move(impl));
	}

};

}
}

#endif

