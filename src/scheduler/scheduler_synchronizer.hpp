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
#ifndef M3BP_SCHEDULER_SCHEDULER_SYNCHRONIZER_HPP
#define M3BP_SCHEDULER_SCHEDULER_SYNCHRONIZER_HPP

#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "m3bp/types.hpp"

namespace m3bp {

class SchedulerSynchronizer {

private:
	std::atomic<bool> m_is_sleeping;
	std::condition_variable m_sleep_condvar;
	std::mutex m_sleep_mutex;

public:
	SchedulerSynchronizer()
		: m_is_sleeping(false)
		, m_sleep_condvar()
		, m_sleep_mutex()
	{ }

	void set_is_sleeping(){
		m_is_sleeping.store(true);
	}
	void reset_is_sleeping(){
		m_is_sleeping.store(false);
	}

	bool notify(){
		if(!m_is_sleeping.load()){ return false; }
		{
			std::unique_lock<std::mutex> lock(m_sleep_mutex);
			if(!m_is_sleeping.load()){ return false; }
			m_is_sleeping.store(false);
		}
		m_sleep_condvar.notify_one();
		return true;
	}

	void wait(){
		std::unique_lock<std::mutex> lock(m_sleep_mutex);
		m_sleep_condvar.wait(lock, [this]() -> bool {
			return !m_is_sleeping.load();
		});
	}

};

}

#endif

