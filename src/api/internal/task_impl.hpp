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
#include <vector>
#include "m3bp/task.hpp"
#include "api/internal/input_reader_impl.hpp"
#include "api/internal/output_writer_impl.hpp"
#include "scheduler/locality.hpp"

namespace m3bp {

class Scheduler;
class MemoryManager;

namespace internal {

struct TaskInput {
	LockedMemoryReference memory_object;

	TaskInput()
		: memory_object(nullptr)
	{ }

	explicit TaskInput(LockedMemoryReference mobj)
		: memory_object(std::move(mobj))
	{ }
};

struct TaskOutput {
	bool has_keys;

	TaskOutput()
		: has_keys(false)
	{ }

	explicit TaskOutput(bool has_keys)
		: has_keys(has_keys)
	{ }
};

class TaskImpl {

private:
	identifier_type m_logical_task_id;
	identifier_type m_physical_task_id;

	ExecutionContext *m_context;
	LogicalTaskBase  *m_process_logical_task;
	Locality          m_current_locality;

	std::vector<TaskInput>  m_inputs;
	std::vector<TaskOutput> m_outputs;

	bool m_is_cancelled;

public:
	TaskImpl()
		: m_logical_task_id()
		, m_physical_task_id()
		, m_context(nullptr)
		, m_process_logical_task(nullptr)
		, m_current_locality()
		, m_inputs()
		, m_outputs()
		, m_is_cancelled(false)
	{ }


	TaskImpl &context(ExecutionContext *context){
		m_context = context;
		return *this;
	}

	TaskImpl &process_logical_task(LogicalTaskBase *task){
		m_process_logical_task = task;
		return *this;
	}

	TaskImpl &current_locality(const Locality &locality){
		m_current_locality = locality;
		return *this;
	}


	identifier_type logical_task_id() const {
		return m_logical_task_id;
	}
	TaskImpl &logical_task_id(identifier_type id){
		m_logical_task_id = id;
		return *this;
	}

	identifier_type physical_task_id() const {
		return m_physical_task_id;
	}
	TaskImpl &physical_task_id(identifier_type id){
		m_physical_task_id = id;
		return *this;
	}


	TaskImpl &input_count(size_type count){
		m_inputs.resize(count);
		return *this;
	}

	InputReader input(identifier_type port_id){
		if(port_id >= m_inputs.size()){
			throw std::out_of_range("Input port is out of range");
		}
		InputReaderImpl reader_impl;
		reader_impl.set_fragment(m_inputs[port_id].memory_object);
		return InputReaderImpl::wrap_impl(std::move(reader_impl));
	}

	TaskImpl &input(identifier_type port_id, TaskInput input){
		assert(port_id < m_inputs.size());
		m_inputs[port_id] = std::move(input);
		return *this;
	}


	TaskImpl &output_count(size_type count){
		m_outputs.resize(count);
		return *this;
	}

	OutputWriter output(identifier_type port_id){
		if(port_id >= m_outputs.size()){
			throw std::out_of_range("Output port is out of range");
		}
		const auto &config = m_context->configuration();
		OutputWriterImpl writer_impl;
		writer_impl
			.context                   (m_context)
			.processor_task            (m_process_logical_task)
			.current_locality          (m_current_locality)
			.output_port               (port_id)
			.has_keys                  (m_outputs[port_id].has_keys)
			.default_buffer_size       (config.default_output_buffer_size())
			.default_records_per_buffer(config.default_records_per_buffer());
		return OutputWriterImpl::wrap_impl(std::move(writer_impl));
	}

	TaskImpl &output(identifier_type port_id, TaskOutput output){
		assert(port_id < m_outputs.size());
		m_outputs[port_id] = std::move(output);
		return *this;
	}


	bool is_cancelled() const {
		return m_is_cancelled;
	}

	TaskImpl &is_cancelled(bool flag){
		m_is_cancelled = flag;
		return *this;
	}


	static TaskImpl &get_impl(Task &task){
		return *task.m_impl;
	}

	static Task wrap_impl(TaskImpl impl){
		return Task(std::move(impl));
	}

};

}
}

