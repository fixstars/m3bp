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
#ifndef M3BP_COMMON_ARRAY_REF_HPP
#define M3BP_COMMON_ARRAY_REF_HPP

#include <cstddef>
#include <stdexcept>

namespace m3bp {

template <typename T>
class ArrayRef {

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
	T *m_first;
	T *m_last;

public:
	ArrayRef() noexcept
		: m_first(nullptr)
		, m_last(nullptr)
	{ }

	ArrayRef(pointer first, pointer last) noexcept
		: m_first(first)
		, m_last(last)
	{ }


	reference at(size_type pos){
		if(pos >= size()){
			throw std::out_of_range("index out of range");
		}
		return m_first[pos];
	}
	const_reference at(size_type pos) const {
		if(pos >= size()){
			throw std::out_of_range("index out of range");
		}
		return m_first[pos];
	}

	reference operator[](size_type pos) noexcept {
		return m_first[pos];
	}
	const_reference operator[](size_type pos) const noexcept {
		return m_first[pos];
	}

	reference front() noexcept { return *m_first; }
	const_reference front() const noexcept { return *m_first; }

	reference back() noexcept { return m_last[-1]; }
	const_reference back() const noexcept { return m_last[-1]; }

	pointer data() noexcept { return m_first; }
	const_pointer data() const noexcept { return m_first; }


	iterator begin() noexcept { return m_first; }
	const_iterator begin() const noexcept { return m_first; }
	const_iterator cbegin() const noexcept { return m_first; }

	iterator end() noexcept { return m_last; }
	const_iterator end() const noexcept { return m_last; }
	const_iterator cend() const noexcept { return m_last; }


	bool empty() const noexcept {
		return m_first == m_last;
	}

	size_type size() const noexcept {
		return static_cast<std::size_t>(m_last - m_first);
	}

};



}

#endif

