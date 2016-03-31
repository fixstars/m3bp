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
#ifndef M3BP_CONTEXT_WORKER_THREADS_HPP
#define M3BP_CONTEXT_WORKER_THREADS_HPP

#include <memory>
#include <thread>
#include <vector>
#include "m3bp/types.hpp"

namespace m3bp {

class ThreadObserverBase;
class ExecutionContext;

class WorkerThread {

public:
	using ThreadObserverPtr = std::shared_ptr<ThreadObserverBase>;

private:
	std::thread m_thread;
	identifier_type m_worker_id;

public:
	WorkerThread();
	explicit WorkerThread(identifier_type worker_id);

	void run(
		ExecutionContext &context,
		std::vector<ThreadObserverPtr> observers);

	void join();

};

}

#endif

