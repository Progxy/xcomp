/*
 * Copyright (C) 2026 TheProgxy <theprogxy@gmail.com>
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

#ifndef _ZLIB_BITSTREAM_H_
#define _ZLIB_BITSTREAM_H_

// -------------------
//  Macros Definition
// -------------------
#define CREATE_BIT_STREAM(data_stream, data_size) (BitStream) { .stream = data_stream, .size = data_size, .byte_pos = 0, .bit_pos = 0, .error = 0 }
#define PRINT_BIT_STREAM_INFO(bit_stream) print_bit_stream_info(#bit_stream, __FILE__, __LINE__, bit_stream)

/* ---------------------------------------------------------------------------------------------------------- */
// ---------
//  Structs
// ---------
typedef enum {
   	BITSTREAM_IO_ERROR = 1
} BitStreamError;

typedef struct PACKED_STRUCT BitStream {
	unsigned char* stream;
	unsigned int size;
	unsigned int byte_pos;
	unsigned char bit_pos;
	unsigned char error;
} BitStream;

/* ---------------------------------------------------------------------------------------------------------- */
// ------------------------
//  Functions Declarations
// ------------------------

/* ---------------------------------------------------------------------------------------------------------- */

static void print_bit_stream_info(const char* name, const char* file, const int line, BitStream* bit_stream) {
	printf(" -- BitStream %s info ( at %s:%d) --\n", name, file, line);
	printf("  -> byte_pos: %u\n", bit_stream -> byte_pos);
	printf("  -> bit_pos: %d\n", bit_stream -> bit_pos);
	printf("  -> size: %u\n", bit_stream -> size);
	printf("  -> error: %u\n", bit_stream -> error);
	if (bit_stream -> byte_pos < bit_stream -> size) {
		printf("  -> current_byte: 0x%X\n", (bit_stream -> stream)[bit_stream -> byte_pos]);
	}
	printf("----------------------------\n");
	return;
}

static unsigned char bitstream_read_next_byte(BitStream* bit_stream) {
	if (bit_stream -> error) return 0;
	
   	bit_stream -> byte_pos += (bit_stream -> bit_pos > 0 && bit_stream -> bit_pos < 7);	
	if (bit_stream -> byte_pos >= bit_stream -> size) {
		bit_stream -> error = 1;
		WARNING_LOG("Bitstream out of bounds byte read");
		PRINT_BIT_STREAM_INFO(bit_stream);
		return 0;
	}

	const unsigned int old_pos = bit_stream -> byte_pos;
	(bit_stream -> byte_pos)++;
	bit_stream -> bit_pos = 0;
	
	return *(bit_stream -> stream + old_pos);
}

static void* bitstream_read_bytes(BitStream* bit_stream, unsigned int size, unsigned int nmemb) {
	const unsigned int tot_size = size * nmemb;
	if (bit_stream -> error) return NULL;
	
   	bit_stream -> byte_pos += (bit_stream -> bit_pos > 0 && bit_stream -> bit_pos < 7);	
	if (bit_stream -> byte_pos + tot_size > bit_stream -> size) {
		bit_stream -> error = 1;
		WARNING_LOG("Bitstream out of bounds bytes read, size: %u, nmemb: %u.", size, nmemb); 
		PRINT_BIT_STREAM_INFO(bit_stream);
		return NULL;
	}

	const unsigned int old_pos = bit_stream -> byte_pos;
	bit_stream -> byte_pos += tot_size;
	bit_stream -> bit_pos = 0;
	
	return (bit_stream -> stream + old_pos);
}

static unsigned char bitstream_read_next_bit(BitStream* bit_stream) {
    if (bit_stream -> error) return 0;
	else if (bit_stream -> byte_pos >= bit_stream -> size) {
		bit_stream -> error = 1;
		WARNING_LOG("Bitstream out of bounds bit read");
		PRINT_BIT_STREAM_INFO(bit_stream);
		return 0;
	}

	unsigned char bit_value = ((bit_stream -> stream)[bit_stream -> byte_pos] >> (bit_stream -> bit_pos)) & 1;
	(bit_stream -> bit_pos)++;
	
	if (bit_stream -> bit_pos > 7) bitstream_read_next_byte(bit_stream);

    return bit_value;
}

static unsigned char bitstream_read_bits(BitStream* bit_stream, unsigned int n_bits, void* data) {
	if (n_bits > sizeof(unsigned long long int) * 8) {
		bit_stream -> error = 1;
		WARNING_LOG("Tried to read more than %lu bits: %u", sizeof(unsigned long long int) * 8, n_bits);
		return 1;
	}

    for (unsigned int i = 0, j = 0; i < n_bits; ++i) {
    	j += (i % 8 == 0) && (i != 0); 
		XCOMP_CAST_PTR(data, unsigned char)[j] += bitstream_read_next_bit(bit_stream) << (i % 8);
		if (bit_stream -> error) return 1;
	}

    return 0;
}

UNUSED_FUNCTION static void skip_to_next_byte(BitStream* bit_stream) {
	if (bit_stream -> error) return;
	else if (bit_stream -> byte_pos >= bit_stream -> size) {
		bit_stream -> error = 1;
		WARNING_LOG("Bitstream out of bound skip.");
		return;
	}

	bit_stream -> bit_pos = 0;
	(bit_stream -> byte_pos)++;

	return;
}

UNUSED_FUNCTION static void deallocate_bit_stream(BitStream* bit_stream) {
	XCOMP_SAFE_FREE(bit_stream -> stream);
	bit_stream -> stream   = NULL;
	bit_stream -> byte_pos = 0;
	bit_stream -> bit_pos  = 0;
	bit_stream -> size     = 0;
	return;
}

#endif //_ZLIB_BITSTREAM_H_

