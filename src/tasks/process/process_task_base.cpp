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
#include <iterator>
#include <cassert>
#include "m3bp/processor_base.hpp"
#include "common/make_unique.hpp"
#include "tasks/process/process_task_base.hpp"
#include "scheduler/scheduler.hpp"
#include "scheduler/locality.hpp"
#include "scheduler/locality_option.hpp"
#include "api/internal/task_impl.hpp"
#include "logging/general_logger.hpp"

namespace m3bp {
namespace {

template <typename Iterator>
std::vector<LockedMemoryReference>
lock_broadcast_inputs(Iterator begin, Iterator end){
	std::vector<LockedMemoryReference> locked(std::distance(begin, end));
	auto unlocked_it = begin;
	auto locked_it = locked.begin();
	while(unlocked_it != end){
		if(*unlocked_it){ *locked_it = unlocked_it->lock(); }
		++locked_it;
		++unlocked_it;
	}
	return locked;
}

}

ProcessLogicalTaskBase::ProcessCommandWrapper::ProcessCommandWrapper(
	ProcessLogicalTaskBase *logical_task,
	std::unique_ptr<ProcessCommandBase> command)
	: PhysicalTaskCommandBase()
	, m_logical_task(logical_task)
	, m_command(std::move(command))
{ }

void ProcessLogicalTaskBase::ProcessCommandWrapper::prepare(
	ExecutionContext &context,
	const Locality &locality)
{
	auto unlocked_inputs = m_logical_task->broadcast_inputs();
	m_broadcast_inputs = lock_broadcast_inputs(
		unlocked_inputs.begin(), unlocked_inputs.end());
	m_command->prepare(context, locality);
}

void ProcessLogicalTaskBase::ProcessCommandWrapper::run(
	ExecutionContext &context,
	const Locality &locality)
{
	m_command->run(
		context, locality, std::move(m_broadcast_inputs));
	m_logical_task->notify_completion(context);
}


class ProcessLogicalTaskBase::GlobalInitializeCommand
	: public PhysicalTaskCommandBase
{
private:
	ProcessLogicalTaskBase *m_logical_task;
	std::vector<LockedMemoryReference> m_broadcast_inputs;

public:
	explicit GlobalInitializeCommand(ProcessLogicalTaskBase *logical_task)
		: PhysicalTaskCommandBase()
		, m_logical_task(logical_task)
	{ }

	virtual void prepare(
		ExecutionContext & /* context */,
		const Locality & /* locality */) override
	{
		auto unlocked_inputs = m_logical_task->broadcast_inputs();
		m_broadcast_inputs = lock_broadcast_inputs(
			unlocked_inputs.begin(), unlocked_inputs.end());
	}

	virtual void run(
		ExecutionContext &context,
		const Locality &locality) override
	{
		m_logical_task->global_initialize(
			context, locality, std::move(m_broadcast_inputs));
	}
};


class ProcessLogicalTaskBase::GlobalFinalizeCommand
	: public PhysicalTaskCommandBase
{
private:
	ProcessLogicalTaskBase *m_logical_task;
	std::vector<LockedMemoryReference> m_broadcast_inputs;

public:
	explicit GlobalFinalizeCommand(ProcessLogicalTaskBase *logical_task)
		: PhysicalTaskCommandBase()
		, m_logical_task(logical_task)
	{ }

	virtual void prepare(
		ExecutionContext & /* context */,
		const Locality & /* locality */) override
	{
		auto unlocked_inputs = m_logical_task->broadcast_inputs();
		m_broadcast_inputs = lock_broadcast_inputs(
			unlocked_inputs.begin(), unlocked_inputs.end());
	}

	virtual void run(
		ExecutionContext &context,
		const Locality &locality) override
	{
		m_logical_task->global_finalize(
			context, locality, std::move(m_broadcast_inputs));
	}
};


class ProcessLogicalTaskBase::ThreadLocalFinalizeCommand
	: public ProcessCommandBase
{
private:
	ProcessLogicalTaskBase *m_logical_task;

public:
	explicit ThreadLocalFinalizeCommand(ProcessLogicalTaskBase *logical_task)
		: ProcessCommandBase()
		, m_logical_task(logical_task)
	{ }

	virtual void run(
		ExecutionContext &context,
		const Locality &locality,
		std::vector<LockedMemoryReference> mobjs) override
	{
		m_logical_task->thread_local_finalize(
			context, locality, std::move(mobjs));
	}
};


class ProcessLogicalTaskBase::ProcessBarrierCommand
	: public PhysicalTaskCommandBase
{
private:
	ProcessLogicalTaskBase *m_logical_task;

public:
	ProcessBarrierCommand(ProcessLogicalTaskBase *logical_task)
		: PhysicalTaskCommandBase()
		, m_logical_task(logical_task)
	{ }

	virtual void run(
		ExecutionContext &context,
		const Locality & /* locality */) override
	{
		m_logical_task->create_thread_local_finalizers(context);
	}
};


ProcessLogicalTaskBase::ProcessLogicalTaskBase()
	: m_processor()
	, m_global_initialized(false)
	, m_thread_local_initialized()
	, m_broadcast_inputs()
	, m_local_queue_mutex()
	, m_remaining_concurrency(0)
	, m_task_queue()
{ }

ProcessLogicalTaskBase::ProcessLogicalTaskBase(
	internal::ProcessorWrapper pw,
	size_type worker_count)
	: m_processor(std::move(pw))
	, m_global_initialized(false)
	, m_thread_local_initialized(worker_count)
	, m_broadcast_inputs(m_processor->input_ports().size())
	, m_local_queue_mutex()
	, m_remaining_concurrency(m_processor->max_concurrency())
	, m_task_queue()
{ }


void ProcessLogicalTaskBase::create_physical_tasks(ExecutionContext &context){
	auto &scheduler = context.scheduler();
	const auto entry_id = scheduler.create_physical_task(
		task_id(),
		make_unique<GlobalInitializeCommand>(this),
		LocalityOption());
	const auto barrier_id = scheduler.create_physical_task(
		task_id(),
		make_unique<ProcessBarrierCommand>(this),
		LocalityOption());
	const auto terminal_id = scheduler.create_physical_task(
		task_id(),
		make_unique<GlobalFinalizeCommand>(this),
		LocalityOption());
	scheduler
		.add_dependency(entry_id, barrier_id)
		.add_dependency(barrier_id, terminal_id);
	entry_task(entry_id);
	barrier_task(barrier_id);
	terminal_task(terminal_id);
}

void ProcessLogicalTaskBase::commit_physical_tasks(ExecutionContext &context){
	auto &scheduler = context.scheduler();
	scheduler.commit_task(entry_task());
	scheduler.commit_task(barrier_task());
	scheduler.commit_task(terminal_task());
}


void ProcessLogicalTaskBase::thread_local_cancel(
	ExecutionContext &context,
	const Locality &locality)
{
	const auto tid = locality.self_thread_id();
	assert(tid < m_thread_local_initialized.size());
	if(!m_thread_local_initialized[tid]){ return; }
	m_thread_local_initialized[tid] = false;
	auto task = create_task_object(
		context,
		std::vector<LockedMemoryReference>(m_broadcast_inputs.size()),
		0, true, locality);
	m_processor->thread_local_finalize(task);
}

void ProcessLogicalTaskBase::global_cancel(ExecutionContext &context){
	if(!m_global_initialized){ return; }
	m_global_initialized = false;
	auto task = create_task_object(
		context,
		std::vector<LockedMemoryReference>(m_broadcast_inputs.size()),
		0, true, Locality());
	m_processor->global_finalize(task);
	m_broadcast_inputs.clear();
}


void ProcessLogicalTaskBase::receive_fragment(
	ExecutionContext &context,
	identifier_type port,
	identifier_type partition,
	MemoryReference mobj)
{
	const auto &input_ports = m_processor->input_ports();
	assert(port < input_ports.size());
	if(input_ports[port].movement() == Movement::BROADCAST){
		assert(!m_global_initialized);
		m_broadcast_inputs[port] = std::move(mobj);
	}else{
		receive_non_broadcast_fragment(
			context, port, partition, std::move(mobj));
	}
}


const ProcessorBase &ProcessLogicalTaskBase::processor() const {
	return *m_processor;
}

ProcessorBase &ProcessLogicalTaskBase::processor(){
	return *m_processor;
}


void ProcessLogicalTaskBase::commit_process_command(
	ExecutionContext &context,
	PhysicalTaskIdentifier task)
{
	auto &scheduler = context.scheduler();
	std::lock_guard<std::mutex> lock(m_local_queue_mutex);
	if(m_remaining_concurrency == 0){
		m_task_queue.push(task);
	}else{
		--m_remaining_concurrency;
		scheduler.commit_task(task);
	}
}


Task ProcessLogicalTaskBase::create_task_object(
	ExecutionContext &context,
	std::vector<LockedMemoryReference> input_buffers,
	identifier_type physical_task_id,
	bool is_cancelled,
	const Locality &locality)
{
	const auto &iports = m_processor->input_ports();
	const auto &oports = m_processor->output_ports();
	internal::TaskImpl task_impl;
	task_impl
		.context             (&context)
		.process_logical_task(this)
		.current_locality    (locality)
		.logical_task_id     (task_id().identifier())
		.physical_task_id    (physical_task_id)
		.input_count         (iports.size())
		.output_count        (oports.size())
		.is_cancelled        (is_cancelled);
	for(identifier_type i = 0; i < iports.size(); ++i){
		if(input_buffers[i]){
			task_impl.input(
				i, internal::TaskInput(std::move(input_buffers[i])));
		}
	}
	for(identifier_type i = 0; i < oports.size(); ++i){
		task_impl.output(i, internal::TaskOutput(oports[i].has_key()));
	}
	return internal::TaskImpl::wrap_impl(std::move(task_impl));
}


void ProcessLogicalTaskBase::global_initialize(
	ExecutionContext &context,
	const Locality &locality,
	std::vector<LockedMemoryReference> mobjs)
{
	if(m_global_initialized){ return; }
	auto task = create_task_object(
		context, std::move(mobjs), 0, false, locality);
	m_processor->global_initialize(task);
	m_global_initialized = true;
	after_global_initialize(context);
}

void ProcessLogicalTaskBase::global_finalize(
	ExecutionContext &context,
	const Locality &locality,
	std::vector<LockedMemoryReference> mobjs)
{
	if(!m_global_initialized){ return; }
	m_global_initialized = false;
	auto task = create_task_object(
		context, std::move(mobjs), 0, false, locality);
	m_processor->global_finalize(task);
	m_broadcast_inputs.clear();
}

void ProcessLogicalTaskBase::thread_local_initialize(
	ExecutionContext &context,
	const Locality &locality,
	std::vector<LockedMemoryReference> mobjs)
{
	const auto tid = locality.self_thread_id();
	assert(tid < m_thread_local_initialized.size());
	if(m_thread_local_initialized[tid]){ return; }
	auto task = create_task_object(
		context, std::move(mobjs), 0, false, locality);
	m_processor->thread_local_initialize(task);
	m_thread_local_initialized[tid] = true;
}

void ProcessLogicalTaskBase::thread_local_finalize(
	ExecutionContext &context,
	const Locality &locality,
	std::vector<LockedMemoryReference> mobjs)
{
	const auto tid = locality.self_thread_id();
	assert(tid < m_thread_local_initialized.size());
	if(!m_thread_local_initialized[tid]){ return; }
	m_thread_local_initialized[tid] = false;
	auto task = create_task_object(
		context, std::move(mobjs), 0, false, locality);
	m_processor->thread_local_finalize(task);
}


void ProcessLogicalTaskBase::create_thread_local_finalizers(
	ExecutionContext &context)
{
	auto &scheduler = context.scheduler();
	const auto thread_count = m_thread_local_initialized.size();
	for(identifier_type i = 0; i < thread_count; ++i){
		if(!m_thread_local_initialized[i]){ continue; }
		const auto finalizer_id = scheduler.create_physical_task(
			task_id(),
			make_unique<ProcessCommandWrapper>(
				this, make_unique<ThreadLocalFinalizeCommand>(this)),
			LocalityOption(i, false));
		scheduler.add_dependency(finalizer_id, terminal_task());
		commit_process_command(context, finalizer_id);
	}
}


void ProcessLogicalTaskBase::notify_completion(ExecutionContext &context){
	auto &scheduler = context.scheduler();
	std::lock_guard<std::mutex> lock(m_local_queue_mutex);
	++m_remaining_concurrency;
	while(!m_task_queue.empty() && m_remaining_concurrency > 0){
		--m_remaining_concurrency;
		scheduler.commit_task(m_task_queue.front());
		m_task_queue.pop();
	}
}


std::vector<MemoryReference> ProcessLogicalTaskBase::broadcast_inputs(){
	return m_broadcast_inputs;
}

}

