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

#include <stdio.h>
#include <stdint.h>

/* -------------------------------------------------------------------------------------------------------- */
// ------------------
//  Macros Functions
// ------------------
#if __has_builtin(__builtin_stdc_rotate_left)
#define xxh_rotl64 __builtin_stdc_rotate_left
#elif __has_builtin(__builtin_stdc_rotate_left) 
#define xxh_rotl64 __builtin_rotateleft64
#else
#define xxh_rotl64(x,r) (((x) << (r)) | ((x) >> (64 - (r))))
#endif

/* -------------------------------------------------------------------------------------------------------- */
// -----------------
//  Constant Values
// -----------------
#define OPTIONAL
static const uint64_t PRIME64_1 = 0x9E3779B185EBCA87ULL; 
static const uint64_t PRIME64_2 = 0xC2B2AE3D27D4EB4FULL;
static const uint64_t PRIME64_3 = 0x165667B19E3779F9ULL;
static const uint64_t PRIME64_4 = 0x85EBCA77C2B2AE63ULL;
static const uint64_t PRIME64_5 = 0x27D4EB2F165667C5ULL;

/* -------------------------------------------------------------------------------------------------------- */
// ------------------------
//  Functions Declarations
// ------------------------

static inline uint64_t xxround(uint64_t acc_n, uint64_t lane_n);
static inline uint64_t merge_accumulator(uint64_t acc, uint64_t acc_n);
uint64_t xxhash64(unsigned char* lane, unsigned int byte_size, OPTIONAL uint64_t seed);

/* -------------------------------------------------------------------------------------------------------- */

static inline uint64_t xxround(uint64_t acc_n, uint64_t lane_n) {
  acc_n += lane_n * PRIME64_2;
  acc_n = xxh_rotl64(acc_n, 31);
  return acc_n * PRIME64_1;
}

static inline uint64_t merge_accumulator(uint64_t acc, uint64_t acc_n) {
  acc ^= xxround(0, acc_n);
  return (acc * PRIME64_1) + PRIME64_4;
}

uint64_t xxhash64(unsigned char* lane, unsigned int byte_size, OPTIONAL uint64_t seed) {
	uint64_t acc = 0;
	unsigned int remaining_size = byte_size;
	if (byte_size >= 32) {
		uint64_t accs[4] = {0};
		accs[0] = seed + PRIME64_1 + PRIME64_2;
		accs[1] = seed + PRIME64_2;
		accs[2] = seed;
		accs[3] = seed - PRIME64_1;
	
		while (remaining_size >= 32) {
			for (unsigned int i = 0; i < 4; ++i, lane += sizeof(uint64_t)) {
				accs[i] = xxround(accs[i], *QCOW_CAST_PTR(lane, uint64_t));
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
      acc ^= xxround(0, *QCOW_CAST_PTR(lane, uint64_t));
      acc = xxh_rotl64(acc, 27) * PRIME64_1 + PRIME64_4;
      lane += sizeof(uint64_t), remaining_size -= sizeof(uint64_t);
	}

	if (remaining_size >= 4) {
      acc ^= (*QCOW_CAST_PTR(lane, uint32_t) * PRIME64_1);
      acc = xxh_rotl64(acc, 23) * PRIME64_2 + PRIME64_3;
	  lane += sizeof(uint32_t), remaining_size -= sizeof(uint32_t);
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
