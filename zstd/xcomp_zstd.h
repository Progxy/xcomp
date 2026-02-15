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

#ifndef _XCOMP_ZSTD_H_
#define _XCOMP_ZSTD_H_

// -----------------
//  Constant Values 
// -----------------
#define ZSTD_SKIPPABLE_FRAME_MAGIC_MIN 0x184D2A50
#define ZSTD_SKIPPABLE_FRAME_MAGIC_MAX 0x184D2A5F
#define ZSTD_FRAME_MAGIC               0xFD2FB528
#define FSE_TABLELOG_ABSOLUTE_MAX      15
#define FSE_MAX_SYMBOL_VALUE           255
#define MAXIMUM_CODE_LENGTH            11
#define NOT_USING_RLE                 (-1) 
#define MAX_LL_CODE                    35
#define MAX_ML_CODE                    52
#define MAX_OL_CODE   	               31
#define U32_BITS                       32
#define MAX_BLOCK_SIZE                (128 * 1024)

/* -------------------------------------------------------------------------------------------------------- */
// -------
//  Enums
// -------
typedef enum PACKED_STRUCT ZstdError {
    ZSTD_NO_ERROR, 
    ZSTD_IO_ERROR,
    ZSTD_RESERVED, 
    ZSTD_TABLE_LOG_TOO_LARGE, 
    ZSTD_CORRUPTED_DATA, 
    ZSTD_MAX_SYMBOL_VALUE_TOO_SMALL, 
    ZSTD_TOO_MANY_LITERALS, 
    ZSTD_CHECKSUM_FAIL,
    ZSTD_INVALID_MAGIC,
    ZSTD_RESERVED_FIELD,
    ZSTD_UNSUPPORTED_FEATURE,
	ZSTD_DECOMPRESSED_SIZE_MISMATCH,
    ZSTD_TODO
} ZstdError;

static const char* zstd_errors_str[] = {
    "ZSTD_NO_ERROR",
    "ZSTD_IO_ERROR",
    "ZSTD_RESERVED", 
    "ZSTD_TABLE_LOG_TOO_LARGE",
    "ZSTD_CORRUPTED_DATA",
    "ZSTD_MAX_SYMBOL_VALUE_TOO_SMALL",
    "ZSTD_TOO_MANY_LITERALS",
    "ZSTD_CHECKSUM_FAIL",
    "ZSTD_INVALID_MAGIC",
    "ZSTD_RESERVED_FIELD", 
    "ZSTD_UNSUPPORTED_FEATURE", 
	"ZSTD_DECOMPRESSED_SIZE_MISMATCH",
    "ZSTD_TODO"
};

typedef enum PACKED_STRUCT LiteralsBlockType { 
	RAW_LITERALS_BLOCK, 
	RLE_LITERALS_BLOCK, 
	COMPRESSED_LITERALS_BLOCK, 
	TREELESS_LITERALS_BLOCK 
} LiteralsBlockType;

typedef enum PACKED_STRUCT BlockType { 
	RAW_BLOCK, 
	RLE_BLOCK, 
	COMPRESSED_BLOCK, 
	RESERVED_TYPE 
} BlockType;

typedef enum PACKED_STRUCT CompressionMode { 
	PREDEFINED_MODE, 
	RLE_MODE, 
	FSE_COMPRESSED_MODE, 
	REPEAT_MODE 
} CompressionMode;

#ifdef _DEBUG
	static const char* literals_blocks_type_str[] = { 
		"RAW_LITERALS_BLOCK", 
		"RLE_LITERALS_BLOCK", 
		"COMPRESSED_LITERALS_BLOCK", 
		"TREELESS_LITERALS_BLOCK" 
	};
	
	static const char* block_types_str[] = { 
		"RAW_BLOCK", 
		"RLE_BLOCK", 
		"COMPRESSED_BLOCK", 
		"RESERVED_TYPE" 
	};
	
	static const char* compression_modes_str[] = { 
		"PREDEFINED_MODE", 
		"RLE_MODE", 
		"FSE_COMPRESSED_MODE", 
		"REPEAT_MODE" 
	};
#endif //_DEBUG

typedef struct PACKED_STRUCT {
	unsigned char dictionary_id_flag: 2;
	unsigned char content_checksum_flag: 1;
	unsigned char reserved: 1;
	unsigned char unused: 1;
	unsigned char single_segment_flag: 1;
	unsigned char frame_content_size_flag: 2;
} FrameHeaderDescriptor;

typedef struct PACKED_STRUCT {
	unsigned char last_block: 1;
	BlockType block_type: 2;
	unsigned int block_size: 21;
} BlockHeader;

#ifdef _XCOMP_BITSTREAM_
#	include "../common/bitstream.h"
#endif //_XCOMP_BITSTREAM_

#include "./zstd_compress.h"
#include "./zstd_decompress.h"

#endif // _XCOMP_ZSTD_H_

