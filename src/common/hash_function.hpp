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
#ifndef M3BP_COMMON_HASH_FUNCTION_HPP
#define M3BP_COMMON_HASH_FUNCTION_HPP

#include <cstdint>

namespace m3bp {

inline identifier_type hash_byte_sequence(
	const void *ptr, size_type length, size_type modulo)
{
	assert(modulo <= 0xffffffffu);
	// MurmurHash3
	const uint8_t *data = static_cast<const uint8_t *>(ptr);
	const auto num_blocks = length / sizeof(uint32_t);
	const uint32_t c1 = 0xcc9e2d51u;
	const uint32_t c2 = 0x1b873593u;
	uint32_t h1 = 0x7a2be187u; // seed

	const uint32_t *blocks = static_cast<const uint32_t *>(ptr);
	for(identifier_type i = 0; i < num_blocks; ++i){
		uint32_t k1 = blocks[i];
		k1 *= c1;
		k1 = (k1 << 15) | (k1 >> (32 - 15));
		k1 *= c2;
		h1 ^= k1;
		h1 = (h1 << 13) | (h1 >> (32 - 13));
		h1 = h1 * 5 + 0xe6546b64;
	}

	const uint8_t *tail_ptr = data + num_blocks * sizeof(uint32_t);
	uint32_t k1 = 0u;
	switch(length & 3){
		case 3:
			k1 ^= static_cast<uint32_t>(tail_ptr[2]) << 16;
		case 2:
			k1 ^= static_cast<uint32_t>(tail_ptr[1]) << 8;
		case 1:
			k1 ^= static_cast<uint32_t>(tail_ptr[0]);
			k1 *= c1;
			k1 = (k1 << 15) | (k1 >> (32 - 15));
			k1 *= c2;
			h1 ^= k1;
	}

	h1 ^= static_cast<uint32_t>(length);
	h1 ^= h1 >> 16;
	h1 *= 0x85ebca6bu;
	h1 ^= h1 >> 13;
	h1 *= 0xc2b2ae35;
	h1 ^= h1 >> 16;
	return (static_cast<uint64_t>(h1) * modulo) >> 32;
}

}

#endif

