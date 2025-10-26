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

#ifndef _XCOMP_LIB_H_
#define _XCOMP_LIB_H_

// -------
//  Enum
// -------
typedef enum CompressionAlgorithm {
	ZSTD,
	ZLIB
} CompressionAlgorithm;

/* -------------------------------------------------------------------------------------------------------- */
// ------------------------
//  Functions Declarations
// ------------------------
/// NOTE: the stream will be always deallocated both in case of failure and success.
/// 	  Furthermore, the function allocates the returned stream of bytes, so that
/// 	  once it's on the hand of the caller, it's responsible to manage that memory.
unsigned char* deflate(unsigned char* stream, unsigned int size, unsigned int* compressed_len, CompressionAlgorithm compression_algorithm, int* err);

/// NOTE: the stream will be always deallocated both in case of failure and success.
/// 	  Furthermore, the function allocates the returned stream of bytes, so that
/// 	  once it's on the hand of the caller, it's responsible to manage that memory.
unsigned char* inflate(unsigned char* stream, unsigned int size, unsigned int* decompressed_len, CompressionAlgorithm compression_algorithm, int* err);

/* -------------------------------------------------------------------------------------------------------- */
unsigned char* deflate(unsigned char* stream, unsigned int size, unsigned int* compressed_len, CompressionAlgorithm compression_algorithm, int* err) {
	if (compression_algorithm == ZLIB) {
		return zlib_deflate(stream, size, compressed_len, err);	
	} else if (compression_algorithm == ZLIB) {
		return zstd_deflate(stream, size, compressed_len, err);	
	}

	*err = UNKNOWN_COMPRESSION_ALGORITHM;
	printf("XCOMP_LIB::ERROR: Unknown compression algorithm.");

	return NULL;
}

unsigned char* inflate(unsigned char* stream, unsigned int size, unsigned int* decompressed_len, CompressionAlgorithm compression_algorithm, int* err) {
	if (compression_algorithm == ZLIB) {
		return zlib_inflate(stream, size, decompressed_len, err);
	} else if (compression_algorithm == ZLIB) {
		return zlib_inflate(stream, size, decompressed_len, err);
	}
	
	*err = UNKNOWN_COMPRESSION_ALGORITHM;
	printf("XCOMP_LIB::ERROR: Unknown compression algorithm.");

	return NULL;
}



#endif //_XCOMP_LIB_H_

