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
#ifndef M3BP_SCHEDULER_CANCELLATION_MANAGER_HPP
#define M3BP_SCHEDULER_CANCELLATION_MANAGER_HPP

#include <mutex>
#include <exception>

namespace m3bp {

class CancellationManager {

private:
	std::mutex m_mutex;
	std::exception_ptr m_exception_ptr;

public:
	CancellationManager() noexcept;

	void notify_exception(std::exception_ptr eptr);
	void rethrow_exception();

	bool is_cancelled() const noexcept;

};

}

#endif

