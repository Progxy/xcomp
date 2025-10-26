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

#ifndef _ZSTD_COMPRESS_H_
#define _ZSTD_COMPRESS_H_

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Resources: zstd <http://github.com/facebook/zstd> (original repo) *
 *                 <https://datatracker.ietf.org/doc/html/rfc8878>   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "./xxhash64.h"
#include "../common/bitstream.h"

// TODO: Implement the compression

// -----------------
//  Constant Values
// -----------------
/* -------------------------------------------------------------------------------------------------------- */
// ---------
//  Structs
// ---------

/* -------------------------------------------------------------------------------------------------------- */
// ------------------------
//  Functions Declarations
// ------------------------

/// NOTE: the stream will be always deallocated both in case of failure and success.
/// 	  Furthermore, the function allocates the returned stream of bytes, so that
/// 	  once it's on the hand of the caller, it's responsible to manage that memory.
unsigned char* zstd_deflate(unsigned char* stream, unsigned int size, unsigned int* compressed_len, int* zstd_err);	

/* -------------------------------------------------------------------------------------------------------- */

unsigned char* zstd_deflate(unsigned char* stream, unsigned int size, unsigned int* compressed_len, int* zstd_err) {
	*compressed_len = 0;
	*zstd_err = ZSTD_TODO;

	printf("size: %u\n", size);

	printf("Implement compression algorithm.\n");

	XCOMP_SAFE_FREE(stream);

	return NULL;
}

#endif //_ZSTD_COMPRESS_H_

