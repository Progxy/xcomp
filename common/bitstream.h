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

#ifndef _BITSTREAM_H_
#define _BITSTREAM_H_

// -------------------
//  Macros Definition
// -------------------
#define IS_EOS(bit_stream) ((bit_stream) -> byte_pos == (bit_stream) -> size)
#define IS_REVERSED_EOS(bit_stream) ((bit_stream) -> byte_pos == 0 && (bit_stream) -> bit_pos == 0)
#define REWIND_BIT_STREAM(bit_stream) ((bit_stream) -> byte_pos = 0, (bit_stream) -> bit_pos = 0, (bit_stream) -> error = 0)
#define CREATE_BIT_STREAM(data_stream, data_size) (BitStream) { .stream = data_stream, .size = data_size, .byte_pos = 0, .bit_pos = 0, .bit_lower_limit = 0, .error = 0 }
#define CREATE_REVERSED_BIT_STREAM(data_stream, data_size, data_lower_limit) (BitStream) { .stream = data_stream, .size = data_size, .byte_pos = data_size - 1, .bit_pos = 8, .bit_lower_limit = data_lower_limit, .error = 0 }
#define PRINT_BIT_STREAM_INFO(bit_stream) DEBUG_LOG("%s: byte_pos: %u, bit_pos: %d, size: %u, error: %u, current_byte: 0x%X.\n", #bit_stream, (bit_stream) -> byte_pos, (bit_stream) -> bit_pos, (bit_stream) -> size, (bit_stream) -> error, ((bit_stream) -> stream)[(bit_stream) -> byte_pos])

#define BITSTREAM_IO_ERROR 1
#define SAFE_NEXT_BIT_READ(bit_stream, value, ...) 									\
	0;																				\
	do {																			\
		value = bitstream_read_next_bit((bit_stream));								\
		 if ((bit_stream) -> error) {												\
	 	 	XCOMP_MULTI_FREE(__VA_ARGS__);												\
		 	WARNING_LOG("An error occurred while reading from the bitstream.\n");	\
		 	return -BITSTREAM_IO_ERROR;												\
		 }																			\
	} while (0)

#define SAFE_BITS_READ(bit_stream, value, nb_bits, ...) 							\
	0;																				\
	do {																			\
		value = bitstream_read_bits((bit_stream), (nb_bits));						\
		 if ((bit_stream) -> error) {												\
	 	 	XCOMP_MULTI_FREE(__VA_ARGS__);												\
		 	WARNING_LOG("An error occurred while reading from the bitstream.\n");	\
		 	return -BITSTREAM_IO_ERROR;												\
		 }																			\
	} while (0)

#define SAFE_BYTE_READ(bit_stream, size, nmemb, var, ...) 							\
	NULL;																			\
	do {																			\
		void* _tmp = bitstream_read_bytes((bit_stream), (size), (nmemb));			\
		 if (_tmp == NULL) {														\
	 	 	XCOMP_MULTI_FREE(__VA_ARGS__);												\
		 	WARNING_LOG("An error occurred while reading from the bitstream.\n");	\
		 	return -BITSTREAM_IO_ERROR;												\
		 }																			\
		 var = _tmp;																\
	} while (0)

#define SAFE_BYTE_READ_WITH_CAST(bit_stream, size, nmemb, type, var, def_val, ...)  \
	def_val;																		\
	do {																			\
		void* _tmp = bitstream_read_bytes((bit_stream), (size), (nmemb));			\
		 if (_tmp == NULL) {														\
	 	 	XCOMP_MULTI_FREE(__VA_ARGS__);												\
		 	WARNING_LOG("An error occurred while reading from the bitstream.\n");	\
		 	return -BITSTREAM_IO_ERROR;												\
		 }																			\
		 var = *XCOMP_CAST_PTR(_tmp, type);												\
	} while (0)

#define SAFE_NEXT_BIT_WRITE(bit_stream, bit, ...) 									\
	do {																			\
		bitstream_write_next_bit((bit_stream), (bit));								\
		if ((bit_stream) -> error) {												\
	 	 	XCOMP_MULTI_FREE(__VA_ARGS__);												\
		 	WARNING_LOG("An error occurred while writing to the bitstream.\n");		\
		 	return -BITSTREAM_IO_ERROR;												\
		 }																			\
	} while (0)

#define SAFE_BIT_WRITE(bit_stream, value, nb_bits, ...) 							\
	do {																			\
		bitstream_write_bits((bit_stream), (value), (nb_bits));						\
		if ((bit_stream) -> error) {												\
	 	 	__VA_ARGS__;															\
		 	WARNING_LOG("An error occurred while writing to the bitstream.\n");		\
		 	return -BITSTREAM_IO_ERROR;												\
		 }																			\
	} while (0)

#define SAFE_REV_BIT_WRITE(bit_stream, value, nb_bits, ...) 						\
	do {																			\
		bitstream_write_bits_reversed((bit_stream), (value), (nb_bits));			\
		if ((bit_stream) -> error) {												\
	 	 	__VA_ARGS__;															\
		 	WARNING_LOG("An error occurred while writing to the bitstream.\n");		\
		 	return -BITSTREAM_IO_ERROR;												\
		 }																			\
	} while (0)

#define SAFE_BYTE_WRITE(bit_stream, size, nmemb, var, ...) 							\
	do {																			\
		bitstream_write_bytes((bit_stream), (size), (nmemb), var);					\
		 if ((bit_stream) -> error) {												\
	 	 	XCOMP_MULTI_FREE(__VA_ARGS__);												\
		 	WARNING_LOG("An error occurred while writing to the bitstream.\n");		\
		 	return -BITSTREAM_IO_ERROR;												\
		 }																			\
	} while (0)

/* ---------------------------------------------------------------------------------------------------------- */
// ---------
//  Structs
// ---------
typedef struct PACKED_STRUCT BitStream {
	unsigned char* stream;
	unsigned int size;
	char bit_lower_limit;
	unsigned int byte_pos;
	char bit_pos;
	unsigned char error;
} BitStream;

/* ---------------------------------------------------------------------------------------------------------- */
// ------------------------
//  Functions Declarations
// ------------------------
static void* bitstream_read_bytes(BitStream* bit_stream, unsigned int size, unsigned int nmemb);
static unsigned char bitstream_read_next_bit(BitStream* bit_stream);
static unsigned long long int bitstream_read_bits(BitStream* bit_stream, unsigned char n_bits);
static void* reversed_bitstream_read_bytes(BitStream* reversed_bit_stream, unsigned int size, unsigned int nmemb);
static unsigned char reversed_bitstream_read_next_bit(BitStream* reversed_bit_stream);
UNUSED_FUNCTION static unsigned long long int reversed_bitstream_read_bits(BitStream* reversed_bit_stream, unsigned char n_bits);
UNUSED_FUNCTION static void bitstream_unread_bit(BitStream* bit_stream);
UNUSED_FUNCTION static void skip_to_next_byte(BitStream* bit_stream);
static void resize_bit_stream(BitStream* bit_stream);
static void bitstream_write_next_bit(BitStream* bit_stream, unsigned char bit);
UNUSED_FUNCTION static void bitstream_write_bits_reversed(BitStream* bit_stream, unsigned long long int bits, unsigned char n_bits);
UNUSED_FUNCTION static void bitstream_write_bits(BitStream* bit_stream, unsigned long long int bits, unsigned char n_bits);
UNUSED_FUNCTION static void deallocate_bit_stream(BitStream* bit_stream);

/* ---------------------------------------------------------------------------------------------------------- */

// TODO: All this functions can be further optimized for performance.
static void* bitstream_read_bytes(BitStream* bit_stream, unsigned int size, unsigned int nmemb) {
	if (bit_stream -> error) {
		WARNING_LOG("Bitstream error bytes.\n"); 
		return NULL;
	}

	unsigned int tot_size = size * nmemb;
	if (bit_stream -> bit_pos != 0) {
		tot_size--;
		(bit_stream -> byte_pos)++;
	}

	if (bit_stream -> byte_pos + tot_size > bit_stream -> size) {
		bit_stream -> error = 1;
		WARNING_LOG("Bitstream gen error bytes, size: %u, nmemb: %u.\n", size, nmemb); 
		PRINT_BIT_STREAM_INFO(bit_stream);
		return NULL;
	}

	bit_stream -> bit_pos = 0;
	unsigned int old_pos = bit_stream -> byte_pos;
	bit_stream -> byte_pos += tot_size;
	return bit_stream -> stream + old_pos;
}

static unsigned char bitstream_read_next_bit(BitStream* bit_stream) {
    if (bit_stream -> error) {
		WARNING_LOG("Bitstream error bits.\n"); 
		return 0;
	}
	
	if (bit_stream -> byte_pos >= bit_stream -> size) {
		bit_stream -> error = 1;
		WARNING_LOG("Gen bit error:\n");
		PRINT_BIT_STREAM_INFO(bit_stream);
		return 0;
	}

	unsigned char bit_value = (bit_stream -> stream[bit_stream -> byte_pos] >> bit_stream -> bit_pos) & 1;
	(bit_stream -> bit_pos)++;

	if (bit_stream -> bit_pos > 7) bitstream_read_bytes(bit_stream, sizeof(unsigned char), 1);

    return bit_value;
}

static unsigned long long int bitstream_read_bits(BitStream* bit_stream, unsigned char n_bits) {
	if (n_bits > sizeof(unsigned long long int) * 8) {
		bit_stream -> error = 1;
		WARNING_LOG("Tried to read more than %lu bits: %u\n", sizeof(unsigned long long int) * 8, n_bits);
		return 0;
	}

	unsigned long long int bits = 0;
    for (unsigned char i = 0; i < n_bits; ++i) {
        bits += bitstream_read_next_bit(bit_stream) << i;
		if (bit_stream -> error) {
			WARNING_LOG("Bitstream error n_bits.\n"); 
			return 0;
		}
    }

    return bits;
}

static void* reversed_bitstream_read_bytes(BitStream* reversed_bit_stream, unsigned int size, unsigned int nmemb) {
	if (reversed_bit_stream -> error) {
		WARNING_LOG("Reversed Bitstream error bytes.\n"); 
		return NULL;
	}

	long int tot_size = size * nmemb;
	if (reversed_bit_stream -> bit_pos != 8 && reversed_bit_stream -> byte_pos > 0) {
		tot_size--;
		(reversed_bit_stream -> byte_pos)--;
	}
	
	if (reversed_bit_stream -> byte_pos - tot_size < 0) {
		reversed_bit_stream -> error = 1;
		WARNING_LOG("Reversed Bitstream gen error bytes, size: %u, nmemb: %u.\n", size, nmemb); 
		PRINT_BIT_STREAM_INFO(reversed_bit_stream);
		return NULL;
	}

	reversed_bit_stream -> bit_pos = 8;
	unsigned int old_pos = reversed_bit_stream -> byte_pos;
	reversed_bit_stream -> byte_pos -= tot_size; 

	return reversed_bit_stream -> stream + old_pos;
}

static unsigned char reversed_bitstream_read_next_bit(BitStream* reversed_bit_stream) {
	if (reversed_bit_stream -> error) {
		WARNING_LOG("Reversed Bitstream error bits.\n"); 
		return 0;
	}
	
	if (reversed_bit_stream -> byte_pos == 0 && (reversed_bit_stream -> bit_pos <= 0 && reversed_bit_stream -> bit_pos >= reversed_bit_stream -> bit_lower_limit)) {
		(reversed_bit_stream -> bit_pos)--;
		return 0;
	} else if (reversed_bit_stream -> byte_pos == 0 && reversed_bit_stream -> bit_pos < reversed_bit_stream -> bit_lower_limit) {
		reversed_bit_stream -> error = 1;
		WARNING_LOG("Reversed Gen bit error:\n");
		PRINT_BIT_STREAM_INFO(reversed_bit_stream);
		return 0;
	} 

	if (reversed_bit_stream -> bit_pos == 0) reversed_bitstream_read_bytes(reversed_bit_stream, sizeof(unsigned char), 1);
	
    unsigned char bit_value = (reversed_bit_stream -> stream[reversed_bit_stream -> byte_pos] >> (reversed_bit_stream -> bit_pos - 1)) & 1;
    (reversed_bit_stream -> bit_pos)--;
	
	return bit_value;
}

UNUSED_FUNCTION static unsigned long long int reversed_bitstream_read_bits(BitStream* reversed_bit_stream, unsigned char n_bits) {
	if (n_bits > sizeof(unsigned long long int) * 8) {
		reversed_bit_stream -> error = 1;
		WARNING_LOG("Tried to read more than %lu bits: %u\n", sizeof(unsigned long long int) * 8, n_bits);
		return 0;
	}

	unsigned long long int bits = 0;
    for (unsigned char i = 0; i < n_bits; ++i) {
        bits += reversed_bitstream_read_next_bit(reversed_bit_stream) << (n_bits - i - 1);
    }

    return bits;
}

UNUSED_FUNCTION static void bitstream_unread_bit(BitStream* bit_stream) {
	if (bit_stream -> error) {
		WARNING_LOG("Bitsream error unread_bit.\n");
		return;
	}

	if (bit_stream -> bit_pos) {
	   	(bit_stream -> bit_pos)--;
		return;
	} else if (bit_stream -> byte_pos) {
		(bit_stream -> byte_pos)--;
		bit_stream -> bit_pos = 7;
		return;
	} 
	
	bit_stream -> error = 1;
	WARNING_LOG("Cannot unread a new bit_stream.\n");

	return;
}

UNUSED_FUNCTION static void skip_to_next_byte(BitStream* bit_stream) {
	if (bit_stream -> error) {
		WARNING_LOG("Bitstream error skip.\n");
		return;
	}

	if (bit_stream -> byte_pos >= bit_stream -> size) {
		bit_stream -> error = 1;
		WARNING_LOG("Bitstream gen error skip.\n");
		return;
	}

	bit_stream -> bit_pos = 0;
	(bit_stream -> byte_pos)++;

	return;
}

static void resize_bit_stream(BitStream* bit_stream) {
	if (bit_stream -> error) {
		WARNING_LOG("BitStream error resize stream.\n");
		return;
	}

	bit_stream -> stream = realloc(bit_stream -> stream, bit_stream -> size * sizeof(unsigned char));
	if (bit_stream -> stream == NULL) {
		WARNING_LOG("Failed to reallocate the stream to %u.\n", bit_stream -> size);
		bit_stream -> error = 1;
		return;
	}
	
	mem_set(bit_stream -> stream + bit_stream -> byte_pos, 0, bit_stream -> size - bit_stream -> byte_pos);

	return;
}

UNUSED_FUNCTION static void bitstream_bit_copy(BitStream* dest_bit_stream, BitStream* src_bit_stream) {
	if (dest_bit_stream -> error || src_bit_stream -> error) {
		WARNING_LOG("BitStream error bit copy.\n");
		return;
	}

	unsigned long long int bits_cnt = src_bit_stream -> byte_pos * 8 + src_bit_stream -> bit_pos;
	for (unsigned long long int i = 0; i < bits_cnt; ++i) {
		bitstream_write_next_bit(dest_bit_stream, (src_bit_stream -> stream)[(i - (i % 8)) / 8] >> (i % 8));
		if (dest_bit_stream -> error) {
			WARNING_LOG("BitStream gen error bit copy.\n");
			return;
		}
	}

	return;
}

UNUSED_FUNCTION static void bitstream_write_bytes(BitStream* bit_stream, unsigned int size, unsigned int nmemb, const void* src) {
	if (bit_stream -> error) {
		WARNING_LOG("BitStream error write next bit.\n");
		return;
	}
	
	if (bit_stream -> bit_pos != 0) {
		bit_stream -> bit_pos = 0;
		(bit_stream -> byte_pos)++;
	}

	// Update the size of the bitstream
	unsigned int original_byte_pos = bit_stream -> byte_pos;
	bit_stream -> byte_pos += size * nmemb;
	
	if (bit_stream -> byte_pos >= bit_stream -> size) {
		bit_stream -> size = bit_stream -> byte_pos;
		resize_bit_stream(bit_stream);
		if (bit_stream -> error) {
			WARNING_LOG("Failed to resize the stream.\n");
			return;
		}
	}

	mem_cpy(bit_stream -> stream + original_byte_pos, src, size * nmemb);

	return;
}

static void bitstream_write_next_bit(BitStream* bit_stream, unsigned char bit) {
	if (bit_stream -> error) {
		WARNING_LOG("BitStream error write next bit.\n");
		return;
	}
	
	if (bit_stream -> bit_pos == 8) {
		bit_stream -> bit_pos = 0;
		(bit_stream -> byte_pos)++;
	}
	
	if (bit_stream -> byte_pos >= bit_stream -> size) {
		bit_stream -> size = bit_stream -> byte_pos + 1;
		resize_bit_stream(bit_stream);
		if (bit_stream -> error) {
			WARNING_LOG("Failed to resize the stream.\n");
			return;
		}
	}

	(bit_stream -> stream)[bit_stream -> byte_pos] |= bit << (bit_stream -> bit_pos);
	(bit_stream -> bit_pos)++;

	return;
}

UNUSED_FUNCTION static void bitstream_write_bits_reversed(BitStream* bit_stream, unsigned long long int bits, unsigned char n_bits) {
	if (n_bits > sizeof(unsigned long long int) * 8) {
		bit_stream -> error = 1;
		WARNING_LOG("Tried to write more than %lu bits: %u\n", sizeof(unsigned long long int) * 8, n_bits);
		return;
	}

	unsigned int mask = 1 << (n_bits - 1);
	for (unsigned char i = 0; i < n_bits; ++i, mask >>= 1) {
		bitstream_write_next_bit(bit_stream, (bits & mask) >> (n_bits - 1 - i));
		if (bit_stream -> error) {
			WARNING_LOG("An error occurred while writing %u bits reversed to the stream.\n", n_bits);
			return;
		}
	}

	return;
}

UNUSED_FUNCTION static void bitstream_write_bits(BitStream* bit_stream, unsigned long long int bits, unsigned char n_bits) {
	if (n_bits > sizeof(unsigned long long int) * 8) {
		bit_stream -> error = 1;
		WARNING_LOG("Tried to write more than %lu bits: %u\n", sizeof(unsigned long long int) * 8, n_bits);
		return;
	}

	unsigned int mask = 1;
	for (unsigned char i = 0; i < n_bits; ++i, mask <<= 1) {
		bitstream_write_next_bit(bit_stream, (bits & mask) >> i);
		if (bit_stream -> error) {
			WARNING_LOG("An error occurred while writing %u bits to the stream.\n", n_bits);
			return;
		}
	}

	return;
}

UNUSED_FUNCTION static void deallocate_bit_stream(BitStream* bit_stream) {
	XCOMP_SAFE_FREE(bit_stream -> stream);
	bit_stream -> stream = NULL;
	bit_stream -> byte_pos = 0;
	bit_stream -> bit_pos = 0;
	bit_stream -> size = 0;
	return;
}

#endif //_BITSTREAM_H_

