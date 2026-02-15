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

int generate_block(BitStream* compressed_bitstream, BitStream* bit_stream, const unsigned char is_last_block, const unsigned int max_block_size, int* zstd_err) {
	BlockHeader block_header = {0};
	block_header.last_block = is_last_block;
	block_header.block_size = MIN(bit_stream -> size - bit_stream -> byte_pos, max_block_size);

	// TODO: Missing support for RLE_BLOCK and COMPRESSED_BLOCK
	block_header.block_type = RAW_BLOCK;

	bitstream_write_bytes(compressed_bitstream, sizeof(BlockHeader), 1, &block_header);
	if (compressed_bitstream -> error) {
		printf("Failed to write the block header.\n");
		*zstd_err = compressed_bitstream -> error;
		return *zstd_err;
	}

	if (block_header.block_type == RAW_BLOCK) {
		unsigned char* uncompressed_stream = bitstream_read_bytes(bit_stream, sizeof(unsigned char), block_header.block_size);
		if (uncompressed_stream == NULL) {
			*zstd_err = bit_stream -> error;
			return *zstd_err;
		}

		bitstream_write_bytes(compressed_bitstream, sizeof(unsigned char), block_header.block_size, uncompressed_stream);
		if (compressed_bitstream -> error) {
			printf("Failed to write the block header.\n");
			*zstd_err = compressed_bitstream -> error;
			return *zstd_err;
		}
	} else {
		*zstd_err = -ZSTD_TODO;
		return *zstd_err;
	}

	return *zstd_err;
}

int generate_frame(BitStream* compressed_bitstream, BitStream* bit_stream, int* zstd_err) {
	unsigned int magic = ZSTD_FRAME_MAGIC;
	bitstream_write_bytes(compressed_bitstream, sizeof(unsigned int), 1, &magic);
	if (compressed_bitstream -> error) {
		printf("Failed to write the frame magic value.\n");
		*zstd_err = compressed_bitstream -> error;
		return *zstd_err;
	}

	FrameHeaderDescriptor frame_header_desc = {0};
	unsigned int frame_content_size_field = 0;
	unsigned long long int frame_content_size = bit_stream -> size - bit_stream -> byte_pos;

	if (frame_content_size < 256) {
		frame_header_desc.frame_content_size_flag = 0;
		frame_content_size_field = 1;
	} else if (frame_content_size < 65792) {
		frame_header_desc.frame_content_size_flag = 1;
		frame_content_size_field = 2;
	} else if (frame_content_size <= 0xFFFFFFFF) {
		frame_header_desc.frame_content_size_flag = 2;
		frame_content_size_field = 4;
	} else {
		frame_header_desc.frame_content_size_flag = 3;
		frame_content_size_field = 8;
	}

	// TODO: This should be either 0 or 1 when support for Sliding Window is added
	frame_header_desc.single_segment_flag = 1; 
	
	// TODO: Add support for checksum
	frame_header_desc.content_checksum_flag = 0;
	
	// TODO: We do not support dictionaries in both decompression and compression
	frame_header_desc.dictionary_id_flag = 0;

	bitstream_write_bytes(compressed_bitstream, sizeof(FrameHeaderDescriptor), 1, &frame_header_desc);
	if (compressed_bitstream -> error) {
		printf("Failed to write the frame header descriptor.\n");
		*zstd_err = compressed_bitstream -> error;
		return *zstd_err;
	}

	// TODO: Missing Window Descriptor field
	// TODO: Dictionary ID field is not written as dictionaries are not supported

	bitstream_write_bytes(compressed_bitstream, frame_content_size_field, 1, &frame_content_size);
	if (compressed_bitstream -> error) {
		printf("Failed to write the frame content size.\n");
		*zstd_err = compressed_bitstream -> error;
		return *zstd_err;
	}

	// TODO: Once Sliding Window support is added than this should be modified
	const unsigned int max_block_size = MAX_BLOCK_SIZE;

	unsigned int block_idx = 0;
	do {
		frame_content_size -= MIN(bit_stream -> size - bit_stream -> byte_pos, max_block_size);
		generate_block(compressed_bitstream, bit_stream, frame_content_size == 0, max_block_size, zstd_err);
		if (*zstd_err) {
			printf("Error while generating block %u\n", block_idx);
			return *zstd_err;
		}
		block_idx++;
	} while (frame_content_size > 0);

	return *zstd_err;
}

unsigned char* zstd_deflate(unsigned char* stream, unsigned int size, unsigned int* compressed_len, int* zstd_err) {
	BitStream bit_stream = CREATE_BIT_STREAM(stream, size);
	BitStream compressed_bitstream = CREATE_EMPTY_BIT_STREAM();
	*compressed_len = 0;

	unsigned int frame_idx = 0;
	do {
		generate_frame(&compressed_bitstream, &bit_stream, zstd_err);
		if (*zstd_err) {
			printf("Error while generating frame %u.\n", frame_idx);
			XCOMP_MULTI_FREE(stream, compressed_bitstream.stream);
			return NULL;
		}
		frame_idx++;
	} while (!IS_EOS(&bit_stream));

	XCOMP_MULTI_FREE(stream);
	*compressed_len = compressed_bitstream.size;

	return compressed_bitstream.stream;
}

#endif //_ZSTD_COMPRESS_H_

