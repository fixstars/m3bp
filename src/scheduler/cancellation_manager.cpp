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
#include "scheduler/cancellation_manager.hpp"
#include "logging/general_logger.hpp"

namespace m3bp {

CancellationManager::CancellationManager() noexcept
	: m_exception_ptr()
{ }

void CancellationManager::notify_exception(std::exception_ptr eptr){
	try{
		std::rethrow_exception(eptr);
	}catch(std::exception &e){
		M3BP_GENERAL_LOG(ERROR) << "An exception is thrown: " << e.what();
		eptr = std::current_exception();
	}catch(...){
		M3BP_GENERAL_LOG(ERROR)
			<< "An exception is thrown: "
			<< "(This exception is not inheriting from std::exception)";
		eptr = std::current_exception();
	}
	if(!m_exception_ptr){
		std::lock_guard<std::mutex> lock(m_mutex);
		if(!m_exception_ptr){
			m_exception_ptr = eptr;
		}
	}
	if(m_exception_ptr != eptr){
		M3BP_GENERAL_LOG(ERROR) << "The last exception is suppressed";
	}
}

void CancellationManager::rethrow_exception(){
	if(m_exception_ptr){
		std::rethrow_exception(m_exception_ptr);
	}
}

bool CancellationManager::is_cancelled() const noexcept {
	return m_exception_ptr != nullptr;
}

}

