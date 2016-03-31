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
#ifndef M3BP_TEST_UTIL_BINARY_UTIL_HPP
#define M3BP_TEST_UTIL_BINARY_UTIL_HPP

#include <string>
#include <utility>
#include "m3bp/types.hpp"
#include "common/hash_function.hpp"

namespace util {

//---------------------------------------------------------------------------
// Metaprogamming utility for pair<T1, T2>
//---------------------------------------------------------------------------
template <typename T>
class is_pair : public std::false_type {
};
template <typename T1, typename T2>
class is_pair<std::pair<T1, T2>> : public std::true_type {
};


//---------------------------------------------------------------------------
// Read an instance of T from pointer
//---------------------------------------------------------------------------
template <typename T>
inline auto read_binary(const void *ptr)
	-> typename std::enable_if<
		!is_pair<T>::value, std::pair<T, m3bp::size_type>>::type
{
	return std::pair<T, m3bp::size_type>(
		*static_cast<const T *>(ptr), sizeof(T));
}

template <>
inline std::pair<std::string, m3bp::size_type>
read_binary<std::string>(const void *ptr){
	const std::string s(static_cast<const char *>(ptr));
	return std::make_pair(s, s.size() + 1);
}

template <typename T>
inline auto read_binary(const void *ptr)
	-> typename std::enable_if<
		is_pair<T>::value, std::pair<T, m3bp::size_type>>::type
{
	using T0 = decltype(T().first);
	using T1 = decltype(T().second);
	const auto p0 = read_binary<T0>(ptr);
	const auto p1 =
		read_binary<T1>(static_cast<const uint8_t *>(ptr) + p0.second);
	return std::make_pair(
		std::make_pair(p0.first, p1.first), p0.second + p1.second);
}


//---------------------------------------------------------------------------
// Write an instance of T to pointer
//---------------------------------------------------------------------------
template <typename T>
inline auto binary_length(const T &)
	-> typename std::enable_if<!is_pair<T>::value, m3bp::size_type>::type
{
	return sizeof(T);
}

inline m3bp::size_type binary_length(const std::string &x){
	return x.size() + 1;
}

template <typename T>
inline auto binary_length(const T &x)
	-> typename std::enable_if<is_pair<T>::value, m3bp::size_type>::type
{
	return binary_length(x.first) + binary_length(x.second);
}

template <typename T>
inline auto write_binary(void *ptr, const T &x)
	-> typename std::enable_if<!is_pair<T>::value>::type
{
	memcpy(ptr, &x, sizeof(T));
}

inline void write_binary(void *ptr, const std::string &x){
	memcpy(ptr, x.c_str(), x.size() + 1);
}

template <typename T>
inline auto write_binary(void *ptr, const T &x)
	-> typename std::enable_if<is_pair<T>::value>::type
{
	const auto first_len = binary_length(x.first);
	write_binary(ptr, x.first);
	write_binary(static_cast<uint8_t *>(ptr) + first_len, x.second);
}

//---------------------------------------------------------------------------
// Compute a hash
//---------------------------------------------------------------------------
template <typename T>
inline uint64_t compute_hash(const T &x, m3bp::size_type modulo){
	const auto len = binary_length(x);
	std::vector<uint8_t> buf(len);
	write_binary(buf.data(), x);
	return m3bp::hash_byte_sequence(buf.data(), len, modulo);
}


//---------------------------------------------------------------------------
// Comparators for byte sequences
//---------------------------------------------------------------------------
template <typename T>
inline int binary_compare(const T &a, const T &b){
	std::vector<uint8_t> a_bytes(binary_length(a));
	std::vector<uint8_t> b_bytes(binary_length(b));
	write_binary(a_bytes.data(), a);
	write_binary(b_bytes.data(), b);
	if(a_bytes == b_bytes){ return 0; }
	return (a_bytes < b_bytes) ? -1 : 1;
}

}

#endif

