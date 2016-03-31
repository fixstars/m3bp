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
#ifndef M3BP_SCHEDULER_LOCALITY_HPP
#define M3BP_SCHEDULER_LOCALITY_HPP

#include "m3bp/types.hpp"

namespace m3bp {

class Locality {

private:
	identifier_type m_thread_id;
	identifier_type m_node_id;

public:
	Locality() noexcept
		: m_thread_id(0)
		, m_node_id(0)
	{ }

	Locality(identifier_type thread_id, identifier_type node_id) noexcept
		: m_thread_id(thread_id)
		, m_node_id(node_id)
	{ }


	identifier_type self_thread_id() const noexcept {
		return m_thread_id;
	}

	identifier_type self_node_id() const noexcept {
		return m_node_id;
	}

};

}

#endif

