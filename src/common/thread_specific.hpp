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
#ifndef M3BP_COMMON_THREAD_SPECIFIC_HPP
#define M3BP_COMMON_THREAD_SPECIFIC_HPP

#include <pthread.h>
#include <stdexcept>

namespace m3bp {

template <typename T>
class ThreadSpecific {

private:
	pthread_key_t m_tsd_key;

	static void tsd_destructor(void *ptr){
		if(ptr){ delete reinterpret_cast<T *>(ptr); }
	}

public:
	ThreadSpecific(){
		if(pthread_key_create(&m_tsd_key, &tsd_destructor) != 0){
			throw std::runtime_error(
				"An error occured on `pthread_key_create()`");
		}
	}
	~ThreadSpecific(){
		pthread_key_delete(m_tsd_key);
	}

	ThreadSpecific(const ThreadSpecific<T> &) = delete;
	ThreadSpecific<T> &operator=(const ThreadSpecific<T> &) = delete;

	T &get(){
		auto ptr = pthread_getspecific(m_tsd_key);
		if(ptr){ return *reinterpret_cast<T *>(ptr); }
		T *created = new T();
		if(pthread_setspecific(m_tsd_key, created)){
			throw std::runtime_error(
				"An error occured on `pthread_setspecific()`");
		}
		return *created;
	}

};

}

#endif

