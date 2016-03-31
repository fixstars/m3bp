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
#include <utility>
#include "m3bp/processor_base.hpp"
#include "api/internal/processor_base_impl.hpp"

namespace m3bp {

ProcessorBase::ProcessorBase()
	: m_impl(new internal::ProcessorBaseImpl())
{ }

ProcessorBase::ProcessorBase(
	const std::vector<InputPort>  &iports,
	const std::vector<OutputPort> &oports)
	: m_impl(new internal::ProcessorBaseImpl(iports, oports))
{ }

ProcessorBase::ProcessorBase(const ProcessorBase &proc)
	: m_impl(new internal::ProcessorBaseImpl(*proc.m_impl))
{ }

ProcessorBase::ProcessorBase(ProcessorBase &&) noexcept = default;

ProcessorBase::~ProcessorBase() = default;


ProcessorBase &ProcessorBase::operator=(const ProcessorBase &proc){
	m_impl.reset(new internal::ProcessorBaseImpl(*proc.m_impl));
	return *this;
}

ProcessorBase &ProcessorBase::operator=(ProcessorBase &&) noexcept = default;


const std::vector<InputPort> &ProcessorBase::input_ports() const {
	return m_impl->input_ports();
}

const std::vector<OutputPort> &ProcessorBase::output_ports() const {
	return m_impl->output_ports();
}


size_type ProcessorBase::task_count() const noexcept {
	return m_impl->task_count();
}

ProcessorBase &ProcessorBase::task_count(size_type new_count){
	m_impl->task_count(new_count);
	return *this;
}


size_type ProcessorBase::max_concurrency() const noexcept {
	return m_impl->max_concurrency();
}

ProcessorBase &ProcessorBase::max_concurrency(size_type new_concurrency){
	m_impl->max_concurrency(new_concurrency);
	return *this;
}

}

