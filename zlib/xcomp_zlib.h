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

// -----------------
//  Constant Values 
// -----------------
#define HF_LITERALS_SIZE   286
#define HF_DISTANCE_SIZE   30
#define HF_TABLE_SIZE      19

/* -------------------------------------------------------------------------------------------------------- */
// -------
//  Enums
// -------
typedef enum PACKED_STRUCT ZlibError {
    ZLIB_NO_ERROR, 
    ZLIB_IO_ERROR, 
    ZLIB_CORRUPTED_DATA, 
    ZLIB_INVALID_LEN_CHECKSUM,
    ZLIB_INVALID_COMPRESSION_TYPE,
    ZLIB_INVALID_DECODED_VALUE, 
    ZLIB_INVALID_LENGTH, 
    ZLIB_TODO 
} ZlibError;

static const char* zlib_errors_str[] = {
    "ZLIB_NO_ERROR", 
    "ZLIB_IO_ERROR", 
    "ZLIB_CORRUPTED_DATA",
    "ZLIB_INVALID_LEN_CHECKSUM",
    "ZLIB_INVALID_COMPRESSION_TYPE",
    "ZLIB_INVALID_DECODED_VALUE",
    "ZLIB_INVALID_LENGTH",
    "ZLIB_TODO"
};

typedef enum PACKED_STRUCT BType { NO_COMPRESSION, COMPRESSED_FIXED_HF, COMPRESSED_DYNAMIC_HF, RESERVED } BType;
static const char* btypes_str[] = { "NO_COMPRESSION", "COMPRESSED_FIXED_HF", "COMPRESSED_DYNAMIC_HF", "RESERVED" };

#include "./zlib_compress.h"
#include "./zlib_decompress.h"

#endif // _XCOMP_ZLIB_H_
