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
#ifndef M3BP_SCHEDULER_LOCALITY_OPTION_HPP
#define M3BP_SCHEDULER_LOCALITY_OPTION_HPP

#include <limits>
#include "m3bp/types.hpp"

namespace m3bp {

class LocalityOption {

private:
	static const identifier_type UNSPECIFIED_ID =
		std::numeric_limits<identifier_type>::max();

	identifier_type m_recommended_worker;
	bool m_is_stealable;

public:
	LocalityOption()
		: m_recommended_worker(UNSPECIFIED_ID)
		, m_is_stealable(true)
	{ }

	explicit LocalityOption(
		identifier_type recommended_worker,
		bool is_stealable = true)
		: m_recommended_worker(recommended_worker)
		, m_is_stealable(is_stealable)
	{ }

	bool has_recommendation() const noexcept {
		return m_recommended_worker != UNSPECIFIED_ID;
	}

	identifier_type recommended_worker() const noexcept {
		return m_recommended_worker;
	}

	bool is_stealable() const noexcept {
		return m_is_stealable;
	}

};

}

#endif

