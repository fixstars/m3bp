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
#include <cassert>
#include "memory/memory_reference.hpp"
#include "memory/memory_object.hpp"
#include "logging/profile_logger.hpp"
#include "logging/profile_event_logger.hpp"

namespace m3bp {

LockedMemoryReference::LockedMemoryReference()
	: m_memory_object()
{ }

LockedMemoryReference::LockedMemoryReference(
	std::shared_ptr<MemoryObject> mobj)
	: m_memory_object(std::move(mobj))
{
	if(m_memory_object){ m_memory_object->lock(); }
}

LockedMemoryReference::LockedMemoryReference(
	const LockedMemoryReference &lmr)
	: m_memory_object(lmr.m_memory_object)
{
	if(m_memory_object){ m_memory_object->lock(); }
}

LockedMemoryReference::~LockedMemoryReference(){
	if(m_memory_object){ m_memory_object->unlock(); }
}


LockedMemoryReference &LockedMemoryReference::operator=(
	const LockedMemoryReference &lmr)
{
	if(m_memory_object){ m_memory_object->unlock(); }
	m_memory_object = lmr.m_memory_object;
	if(m_memory_object){ m_memory_object->lock(); }
	return *this;
}

LockedMemoryReference &LockedMemoryReference::operator=(
	LockedMemoryReference &&lmr) noexcept
{
	if(m_memory_object){ m_memory_object->unlock(); }
	m_memory_object = std::move(lmr.m_memory_object);
	return *this;
}


identifier_type LockedMemoryReference::identifier() const noexcept {
	assert(m_memory_object);
	return m_memory_object->identifier();
}

identifier_type LockedMemoryReference::locality() const noexcept {
	assert(m_memory_object);
	return m_memory_object->locality();
}


const void *LockedMemoryReference::pointer() const {
	assert(m_memory_object);
	return m_memory_object->pointer();
}

void *LockedMemoryReference::pointer(){
	assert(m_memory_object);
	return m_memory_object->pointer();
}



MemoryReference::MemoryReference()
	: m_memory_object()
	, m_identifier(NULL_IDENTIFIER)
{ }

MemoryReference::MemoryReference(std::shared_ptr<MemoryObject> mobj)
	: m_memory_object(std::move(mobj))
	, m_identifier(m_memory_object
		? m_memory_object->identifier()
		: NULL_IDENTIFIER)
{ }

MemoryReference::MemoryReference(const LockedMemoryReference &lmr)
	: m_memory_object(lmr.m_memory_object)
	, m_identifier(lmr.identifier())
{ }

MemoryReference::MemoryReference(LockedMemoryReference &&lmr) noexcept
	: m_memory_object(std::move(lmr.m_memory_object))
	, m_identifier(m_memory_object
		? m_memory_object->identifier()
		: NULL_IDENTIFIER)
{
	if(m_memory_object){ m_memory_object->unlock(); }
}

MemoryReference::MemoryReference(MemoryReference &&mr) noexcept
	: m_memory_object(std::move(mr.m_memory_object))
	, m_identifier(mr.m_identifier)
{
	mr.m_identifier = NULL_IDENTIFIER;
}


MemoryReference &MemoryReference::operator=(MemoryReference &&mr) noexcept {
	m_memory_object = std::move(mr.m_memory_object);
	m_identifier = mr.m_identifier;
	mr.m_identifier = NULL_IDENTIFIER;
	return *this;
}


size_type MemoryReference::size() const noexcept {
	assert(m_memory_object);
	return m_memory_object->size();
}

identifier_type MemoryReference::locality() const noexcept {
	assert(m_memory_object);
	return m_memory_object->locality();
}


LockedMemoryReference MemoryReference::lock(){
	auto &event_logger = ProfileLogger::thread_local_logger();
	event_logger.log_lock_memory(*this);
	return LockedMemoryReference(m_memory_object);
}

}

