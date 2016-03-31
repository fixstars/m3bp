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
#ifndef M3BP_TASKS_LOGICAL_TASK_IDENTIFIER_HPP
#define M3BP_TASKS_LOGICAL_TASK_IDENTIFIER_HPP

#include <limits>
#include "m3bp/types.hpp"

namespace m3bp {

class LogicalTaskIdentifier {

private:
	static const identifier_type INVALID_ID =
		std::numeric_limits<identifier_type>::max();

	identifier_type m_identifier;

public:
	LogicalTaskIdentifier()
		: m_identifier(INVALID_ID)
	{ }

	explicit LogicalTaskIdentifier(identifier_type id)
		: m_identifier(id)
	{ }

	explicit operator bool() const noexcept {
		return m_identifier != INVALID_ID;
	}

	bool operator==(const LogicalTaskIdentifier &id) const noexcept {
		return m_identifier == id.m_identifier;
	}
	bool operator!=(const LogicalTaskIdentifier &id) const noexcept {
		return m_identifier != id.m_identifier;
	}

	identifier_type identifier() const noexcept {
		return m_identifier;
	}

};

}

namespace std {

template <>
struct hash<m3bp::LogicalTaskIdentifier> {
	size_t operator()(const m3bp::LogicalTaskIdentifier &id) const {
		return id.identifier();
	}
};

}

#endif

