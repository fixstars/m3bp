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
#ifndef M3BP_TASKS_MSD_RADIX_SORT_HPP
#define M3BP_TASKS_MSD_RADIX_SORT_HPP

#include <cstdint>
#include "m3bp/types.hpp"

namespace m3bp {
namespace {

using cache_type = uint64_t;
static const int RADIX_SORT_THRESHOLD = (1 << 8);
static const int SUPER_ALPHABET_THRESHOLD = (1 << 16);

inline uint16_t bswap(uint16_t x){
	return __builtin_bswap16(x);
}
inline uint32_t bswap(uint32_t x){
	return __builtin_bswap32(x);
}
inline uint64_t bswap(uint64_t x){
	return __builtin_bswap64(x);
}

inline size_type get_record_length(const uint8_t *ptr){
	return reinterpret_cast<const size_type *>(ptr)[0];
}
inline size_type get_key_length(const uint8_t *ptr){
	return reinterpret_cast<const size_type *>(ptr)[1];
}
inline const uint8_t *get_key_pointer(const uint8_t *ptr){
	return ptr + sizeof(size_type) * 2;
}
inline const uint8_t *get_value_pointer(const uint8_t *ptr){
	return ptr + sizeof(size_type) * 2 + get_key_length(ptr);
}

template <typename T>
inline T get_block(const uint8_t *ptr, size_t k){
	const auto key_len = get_key_length(ptr);
	const auto key_ptr = get_key_pointer(ptr) + k;
	T block = 0;
	if(key_len - k >= sizeof(T)){
		block = bswap(*reinterpret_cast<const T *>(key_ptr));
	}else{
		for(size_type i = 0; i < sizeof(T); ++i){
			block <<= 8;
			if(k + i < key_len){
				block |= key_ptr[i];
			}
		}
	}
	return block;
}

template <typename T>
T median(T a, T b, T c){
	return
		a + b + c - std::min(std::min(a, b), c) - std::max(std::max(a, b), c);
}

void quick_sort(cache_type *cache, const uint8_t **pointers, size_t n){
	if(n <= 1){
		return;
	}else if(n == 2){
		if(cache[0] > cache[1]){
			std::swap(cache[0], cache[1]);
			std::swap(pointers[0], pointers[1]);
		}
	}else{
		const auto pivot = median(cache[0], cache[n / 2], cache[n - 1]);
		size_type l = 0, r = n - 1;
		while(true){
			while(cache[l] < pivot){
				++l;
			}
			while(cache[r] > pivot){
				--r;
			}
			if(l >= r){
				break;
			}
			std::swap(cache[l], cache[r]);
			std::swap(pointers[l], pointers[r]);
			++l;
			--r;
		}
		quick_sort(cache, pointers, l);
		quick_sort(cache + r + 1, pointers + r + 1, n - (r + 1));
	}
}

void msd_radix_sort(
	uint8_t *equals_to_left,
	cache_type *cache, const uint8_t **pointers,
	cache_type *cache_work, const uint8_t **pointers_work,
	size_type n, size_type depth = 0, bool swapped = false)
{
	if(n == 0){
		return;
	}
	if(depth % sizeof(cache_type) == 0){
		bool is_finished = true;
		for(size_type i = 0; i < n; ++i){
			const auto key_length = get_key_length(pointers[i]);
			cache_work[i] = get_block<cache_type>(pointers[i], depth);
			if(depth < key_length){ is_finished = false; }
		}
		if(is_finished){
			for(size_type i = 1; i < n; ++i){
				if(cache[i] == cache[i - 1]){
					equals_to_left[i] = true;
				}
			}
			return;
		}else{
			for(size_type i = 0; i < n; ++i){
				cache[i] = cache_work[i];
			}
		}
	}
	if(n < RADIX_SORT_THRESHOLD){
		if(swapped){
			for(size_type i = 0; i < n; ++i){
				cache_work[i] = cache[i];
				pointers_work[i] = pointers[i];
			}
			std::swap(cache, cache_work);
			std::swap(pointers, pointers_work);
		}
		quick_sort(cache, pointers, n);
		const auto next_depth =
			(depth + sizeof(cache_type)) & ~(sizeof(cache_type) - 1);
		for(size_type i = 0; i < n; ){
			size_type j = i + 1;
			while(j < n && cache[i] == cache[j]){
				++j;
			}
			if(j - i >= 2){
				msd_radix_sort(
					equals_to_left + i, cache + i, pointers + i,
					cache_work + i, pointers_work + i, j - i, next_depth);
			}
			i = j;
		}
/*
	}else if(n >= SUPER_ALPHABET_THRESHOLD){
		const size_type BUCKET_SIZE = (1 << 16);
		std::vector<size_type> bins(BUCKET_SIZE);
		const auto shift =
			(sizeof(cache_type) - 2 - depth % sizeof(cache_type)) * 8;
		for(size_type i = 0; i < n; ++i){
			++bins[(cache[i] >> shift) & (BUCKET_SIZE - 1)];
		}
		for(size_type i = 0, s = 0; i < BUCKET_SIZE; ++i){
			const auto t = bins[i];
			bins[i] = s;
			s += t;
		}
		for(size_type i = 0; i < n; ++i){
			const auto p = bins[(cache[i] >> shift) & (BUCKET_SIZE - 1)]++;
			cache_work[p] = cache[i];
			pointers_work[p] = pointers[i];
		}
		for(size_type i = 0, head = 0; i < BUCKET_SIZE; ++i){
			const auto tail = bins[i];
			msd_radix_sort(
				equals_to_left + head,
				cache_work + head, pointers_work + head,
				cache + head, pointers + head,
				tail - head, depth + 2, !swapped);
			head = tail;
		}
*/
	}else{
		const size_type BUCKET_SIZE = (1 << 8);
		std::array<size_type, BUCKET_SIZE> bins;
		std::fill(bins.begin(), bins.end(), 0);
		const auto shift =
			(sizeof(cache_type) - 1 - depth % (sizeof(cache_type))) * 8;
		for(size_type i = 0; i < n; ++i){
			++bins[(cache[i] >> shift) & (BUCKET_SIZE - 1)];
		}
		for(size_type i = 0, s = 0; i < BUCKET_SIZE; ++i){
			const auto t = bins[i];
			bins[i] = s;
			s += t;
		}
		for(size_type i = 0; i < n; ++i){
			const auto p = bins[(cache[i] >> shift) & (BUCKET_SIZE - 1)]++;
			cache_work[p] = cache[i];
			pointers_work[p] = pointers[i];
		}
		for(size_type i = 0, head = 0; i < BUCKET_SIZE; ++i){
			const auto tail = bins[i];
			msd_radix_sort(
				equals_to_left + head,
				cache_work + head, pointers_work + head,
				cache + head, pointers + head,
				tail - head, depth + 1, !swapped);
			head = tail;
		}
	}
}

}
}

#endif

