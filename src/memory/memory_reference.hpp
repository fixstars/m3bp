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
#ifndef M3BP_MEMORY_MEMORY_REFERENCE_HPP
#define M3BP_MEMORY_MEMORY_REFERENCE_HPP

#include <memory>
#include <limits>
#include <cassert>
#include "m3bp/types.hpp"

namespace m3bp {

class MemoryObject;
class MemoryReference;

class LockedMemoryReference {

private:
	friend class MemoryReference;

	std::shared_ptr<MemoryObject> m_memory_object;

public:
	LockedMemoryReference();
	explicit LockedMemoryReference(std::shared_ptr<MemoryObject> mobj);
	LockedMemoryReference(const LockedMemoryReference &lmr);
	LockedMemoryReference(LockedMemoryReference &&) noexcept = default;
	~LockedMemoryReference();

	LockedMemoryReference &operator=(const LockedMemoryReference &lmr);
	LockedMemoryReference &operator=(LockedMemoryReference &&lmr) noexcept;

	explicit operator bool() const noexcept {
		return static_cast<bool>(m_memory_object);
	}

	identifier_type identifier() const noexcept;

	identifier_type locality() const noexcept;

	const void *pointer() const;
	void *pointer();

};

class MemoryReference {

private:
	static const auto NULL_IDENTIFIER =
		std::numeric_limits<identifier_type>::max();

	std::shared_ptr<MemoryObject> m_memory_object;
	identifier_type m_identifier;

public:
	MemoryReference();
	explicit MemoryReference(std::shared_ptr<MemoryObject> mobj);
	explicit MemoryReference(const LockedMemoryReference &lmr);
	explicit MemoryReference(LockedMemoryReference &&lmr) noexcept;
	MemoryReference(const MemoryReference &) = default;
	MemoryReference(MemoryReference &&mr) noexcept;

	MemoryReference &operator=(const MemoryReference &) = default;
	MemoryReference &operator=(MemoryReference &&mr) noexcept;

	explicit operator bool() const noexcept {
		return m_identifier != NULL_IDENTIFIER;
	}

	size_type size() const noexcept;

	identifier_type identifier() const noexcept {
		return m_identifier;
	}

	identifier_type locality() const noexcept;

	LockedMemoryReference lock();

};

}

#endif

