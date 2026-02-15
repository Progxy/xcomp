/*
 * Copyright (C) 2025 TheProgxy <theprogxy@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _XXHASH64_H_
#define _XXHASH64_H_

/* -------------------------------------------------------------------------------------------------------- */
// ------------------
//  Macros Functions
// ------------------
#if __has_builtin(__builtin_stdc_rotate_left)
	#define xxh_rotl64 __builtin_stdc_rotate_left
#elif __has_builtin(__builtin_rotateleft64) 
	#define xxh_rotl64 __builtin_rotateleft64
#else
	#define xxh_rotl64(x,r) (((x) << (r)) | ((x) >> (64 - (r))))
#endif

#define OPTIONAL

typedef unsigned int       u32;
typedef unsigned long long u64;

#define STATIC_ASSERT          _Static_assert
STATIC_ASSERT(sizeof(u32)  == 4,  "u32 must be 4 bytes");
STATIC_ASSERT(sizeof(u64)  == 8,  "u64 must be 8 bytes");
 
/* -------------------------------------------------------------------------------------------------------- */
// -----------------
//  Constant Values
// -----------------
static const u64 PRIME64_1 = 0x9E3779B185EBCA87ULL; 
static const u64 PRIME64_2 = 0xC2B2AE3D27D4EB4FULL;
static const u64 PRIME64_3 = 0x165667B19E3779F9ULL;
static const u64 PRIME64_4 = 0x85EBCA77C2B2AE63ULL;
static const u64 PRIME64_5 = 0x27D4EB2F165667C5ULL;

/* -------------------------------------------------------------------------------------------------------- */
// ------------------------
//  Functions Declarations
// ------------------------

static inline u64 xxround(u64 acc_n, u64 lane_n);
static inline u64 merge_accumulator(u64 acc, u64 acc_n);
u64 xxhash64(unsigned char* lane, unsigned int byte_size, OPTIONAL u64 seed);

/* -------------------------------------------------------------------------------------------------------- */

static inline u64 xxround(u64 acc_n, u64 lane_n) {
  acc_n += lane_n * PRIME64_2;
  acc_n = xxh_rotl64(acc_n, 31);
  return acc_n * PRIME64_1;
}

static inline u64 merge_accumulator(u64 acc, u64 acc_n) {
  acc ^= xxround(0, acc_n);
  return (acc * PRIME64_1) + PRIME64_4;
}

u64 xxhash64(unsigned char* lane, unsigned int byte_size, OPTIONAL u64 seed) {
	u64 acc = 0;
	unsigned int remaining_size = byte_size;
	if (byte_size >= 32) {
		u64 accs[4] = {0};
		accs[0] = seed + PRIME64_1 + PRIME64_2;
		accs[1] = seed + PRIME64_2;
		accs[2] = seed;
		accs[3] = seed - PRIME64_1;
	
		while (remaining_size >= 32) {
			for (unsigned int i = 0; i < 4; ++i, lane += sizeof(u64)) {
				accs[i] = xxround(accs[i], *XCOMP_CAST_PTR(lane, u64));
			}
			remaining_size -= 32; 
		}

		acc = xxh_rotl64(accs[0], 1) + xxh_rotl64(accs[1], 7) + xxh_rotl64(accs[2], 12) + xxh_rotl64(accs[3], 18);
		acc = merge_accumulator(acc, accs[0]);
		acc = merge_accumulator(acc, accs[1]);
		acc = merge_accumulator(acc, accs[2]);
		acc = merge_accumulator(acc, accs[3]);
	} else acc = seed + PRIME64_5; 

 	acc += byte_size;

	while (remaining_size >= 8) {
      acc ^= xxround(0, *XCOMP_CAST_PTR(lane, u64));
      acc = xxh_rotl64(acc, 27) * PRIME64_1 + PRIME64_4;
      lane += sizeof(u64), remaining_size -= sizeof(u64);
	}

	if (remaining_size >= 4) {
      acc ^= (*XCOMP_CAST_PTR(lane, u32) * PRIME64_1);
      acc = xxh_rotl64(acc, 23) * PRIME64_2 + PRIME64_3;
	  lane += sizeof(u32), remaining_size -= sizeof(u32);
	}

	while (remaining_size > 0) {
      acc ^= (*lane++) * PRIME64_5;
      acc = xxh_rotl64(acc, 11) * PRIME64_1;
	  remaining_size--;
	}	
		
	acc ^= (acc >> 33);
	acc *= PRIME64_2;
	acc ^= (acc >> 29);
	acc *= PRIME64_3;
	acc ^= (acc >> 32);

	return acc;
}

#endif //_XXHASH64_H_
