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
#ifndef M3BP_COMMON_NONCOPYABLE_VECTOR_HPP
#define M3BP_COMMON_NONCOPYABLE_VECTOR_HPP

#include <memory>

namespace m3bp {

template <typename T>
class NoncopyableVector {

public:
	using value_type      = T;
	using size_type       = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference       = value_type &;
	using const_reference = const value_type &;
	using pointer         = value_type *;
	using const_pointer   = const value_type *;
	using iterator        = value_type *;
	using const_iterator  = const value_type *;

private:
	size_type m_size;
	std::unique_ptr<value_type[]> m_data;

public:
	NoncopyableVector() noexcept
		: m_size(0)
		, m_data(nullptr)
	{ }

	NoncopyableVector(size_type n)
		: m_size(n)
		, m_data(new value_type[n])
	{ }


	NoncopyableVector(const NoncopyableVector<T> &) = delete;
	NoncopyableVector(NoncopyableVector<T> &&) = default;

	NoncopyableVector<T> &operator=(const NoncopyableVector<T> &) = delete;
	NoncopyableVector<T> &operator=(NoncopyableVector<T> &&) = default;


	reference at(size_type pos){
		if(pos >= size()){
			throw std::out_of_range("index out of range");
		}
		return m_data[pos];
	}
	const_reference at(size_type pos) const {
		if(pos >= size()){
			throw std::out_of_range("index out of range");
		}
		return m_data[pos];
	}

	reference operator[](size_type pos) noexcept {
		return m_data[pos];
	}
	const_reference operator[](size_type pos) const noexcept {
		return m_data[pos];
	}

	reference front() noexcept { return m_data[0]; }
	const_reference front() const noexcept { return m_data[0]; }

	reference back() noexcept { return m_data[m_size - 1]; }
	const_reference back() const noexcept { return m_data[m_size - 1]; }

	pointer data() noexcept { return m_data.get(); }
	const_pointer data() const noexcept { return m_data.get(); }


	iterator begin() noexcept { return m_data.get(); }
	const_iterator begin() const noexcept { return m_data.get(); }
	const_iterator cbegin() const noexcept { return m_data.get(); }

	iterator end() noexcept { return m_data.get() + m_size; }
	const_iterator end() const noexcept { return m_data.get() + m_size; }
	const_iterator cend() const noexcept { return m_data.get() + m_size; }


	bool empty() const noexcept {
		return m_size == 0;
	}

	size_type size() const noexcept {
		return m_size;
	}

};

}

#endif

