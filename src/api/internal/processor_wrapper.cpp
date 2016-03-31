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
#include "m3bp/internal/processor_wrapper.hpp"

namespace m3bp {
namespace internal {

ProcessorWrapper::ProcessorWrapper()
	: m_typed_wrapper()
{ }

ProcessorWrapper::ProcessorWrapper(const ProcessorWrapper &pw)
	: m_typed_wrapper(pw.m_typed_wrapper->clone())
{ }

ProcessorWrapper::ProcessorWrapper(ProcessorWrapper &&) = default;

ProcessorWrapper &ProcessorWrapper::operator=(const ProcessorWrapper &pw){
	m_typed_wrapper.reset(pw.m_typed_wrapper->clone());
	return *this;
}

ProcessorWrapper &ProcessorWrapper::operator=(ProcessorWrapper &&) = default;


const ProcessorBase *ProcessorWrapper::operator->() const {
	return &m_typed_wrapper->processor();
}

ProcessorBase *ProcessorWrapper::operator->(){
	return &m_typed_wrapper->processor();
}

const ProcessorBase &ProcessorWrapper::operator*() const {
	return m_typed_wrapper->processor();
}

ProcessorBase &ProcessorWrapper::operator*(){
	return m_typed_wrapper->processor();
}

}
}

