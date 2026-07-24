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

#ifndef _XCOMP_ZLIB_H_
#define _XCOMP_ZLIB_H_

/* -------------------------------------------------------------------------------------------------------- */
// -------
//  Enums
// -------
typedef enum {
	WINDOW_SIZE                     = 0x8000,
	HF_LITERALS_SIZE                = 286,
	HF_DISTANCE_SIZE                = 30,
	HF_TABLE_SIZE                   = 19,
	BLOCK_DELIMITER                 = 256,
	MAX_HF_SIZE                     = 288,
	FIXED_HF_LITERAL_MAX_BIT_LENGTH = 9
} ZLIBConstants;

typedef enum PACKED_STRUCT ZlibError {
    ZLIB_NO_ERROR, 
    ZLIB_IO_ERROR, 
    ZLIB_CORRUPTED_DATA, 
    ZLIB_INVALID_LEN_CHECKSUM,
    ZLIB_INVALID_ADLER_CHECKSUM,
    ZLIB_INVALID_COMPRESSION_TYPE,
    ZLIB_INVALID_DECODED_VALUE, 
    ZLIB_INVALID_LENGTH, 
	ZLIB_INVALID_COMPRESSION_METHOD,
	ZLIB_INVALID_WINDOW_SIZE,
	ZLIB_DICTIONARY_NOT_SUPPORTED,
	ZLIB_INVALID_CHECKSUM,
    ZLIB_TODO 
} ZlibError;

static const char* zlib_errors_str[] = {
    "ZLIB_NO_ERROR", 
    "ZLIB_IO_ERROR", 
    "ZLIB_CORRUPTED_DATA",
    "ZLIB_INVALID_LEN_CHECKSUM",
    "ZLIB_INVALID_ADLER_CHECKSUM",
    "ZLIB_INVALID_COMPRESSION_TYPE",
    "ZLIB_INVALID_DECODED_VALUE",
    "ZLIB_INVALID_LENGTH",
	"ZLIB_INVALID_COMPRESSION_METHOD",
	"ZLIB_INVALID_WINDOW_SIZE",
	"ZLIB_DICTIONARY_NOT_SUPPORTED",
	"ZLIB_INVALID_CHECKSUM",
    "ZLIB_TODO"
};

typedef enum PACKED_STRUCT BType { 
	NO_COMPRESSION, 
	COMPRESSED_FIXED_HF, 
	COMPRESSED_DYNAMIC_HF, 
	RESERVED 
} BType;

static const char* btypes_str[] = { 
	"NO_COMPRESSION", 
	"COMPRESSED_FIXED_HF", 
	"COMPRESSED_DYNAMIC_HF", 
	"RESERVED" 
};

/* -------------------------------------------------------------------------------------------------------- */
// ---------
//  Structs 
// ---------
typedef struct {
	unsigned char* data;
	unsigned int size;
	unsigned int pos;
} zlib_buffer_t; 

typedef struct PACKED_STRUCT {
	unsigned char is_final:   1;
	BType compression_method: 2;
	unsigned char padding:    5;
} zlib_block_t;

typedef struct {
	unsigned short int hlit;
	unsigned short int hdist;
	unsigned short int hclen;
} dhf_header_t;

typedef struct {
	unsigned char compression_method;
    unsigned int  window_size;
    unsigned char preset_dictionary;
    unsigned char compression_level;
} zlib_header_t;

/* -------------------------------------------------------------------------------------------------------- */
#ifdef _XCOMP_BITSTREAM_
#	include "./zlib_bitstream.h"
#endif //_XCOMP_BITSTREAM_

static inline unsigned int __adler_crc(const unsigned char* data, const unsigned int size, unsigned int adler_reg) {
    const unsigned int prime = 65521;
	unsigned int low  = adler_reg & 0xFFFF;
	unsigned int high = (adler_reg >> 16) & 0xFFFF;
	for (unsigned int i = 0; i < size; ++i) {
		low  = (low + data[i]) % prime;
		high = (low + high) % prime;
	}
	return ((high << 16) | low);
}

#include "./zlib_compress.h"
#include "./zlib_decompress.h"

#endif // _XCOMP_ZLIB_H_
