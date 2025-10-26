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

#ifndef _ZLIB_DECOMPRESS_H_
#define _ZLIB_DECOMPRESS_H_

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Resources: deflate <https://www.ietf.org/rfc/rfc1951.txt> *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// -------------------
//  Macros Definition
// -------------------
#define IS_FIXED_LITERALS 2

#define DEALLOCATE_TABLES(hf_a, hf_b)	\
	do {      							\
		deallocate_hf_table(hf_a);      \
		deallocate_hf_table(hf_b);      \
	} while (FALSE)      		

#define MAX_VALUE(max, vec, len) 							\
	0;														\
	do { 													\
		for (unsigned short int i = 0; i < (len); ++i) {	\
			(max) = ((max) < (vec)[i]) ? (vec)[i] : (max);	\
		}													\
	} while (FALSE)

#define BITSTREAM_SAFE_CHECK(bit_stream, literals_hf, distance_hf) 	\
	if ((bit_stream) -> error) {									\
		DEALLOCATE_TABLES((literals_hf), (distance_hf)); 			\
		return -ZLIB_IO_ERROR;										\
	}

#define READ_BLOCK_HEADER(bit_stream, zlib_err, is_final, compression_method) 					\
	do {																						\
		is_final = bitstream_read_next_bit(bit_stream);											\
		if ((bit_stream) -> error) {															\
			(zlib_err) = -ZLIB_IO_ERROR;														\
			return ((unsigned char*) "An error occurred while reading from the bitstream.\n");	\
		}																						\
		compression_method = bitstream_read_bits((bit_stream), 2);								\
		if ((bit_stream) -> error) {															\
			(zlib_err) = -ZLIB_IO_ERROR;														\
			return ((unsigned char*) "An error occurred while reading from the bitstream.\n");	\
		}																						\
	} while (FALSE)

/* ---------------------------------------------------------------------------------------------------------- */
// ---------
//  Structs
// ---------
typedef struct HFTable {
    unsigned short int** values;
    unsigned short int* min_codes;
    unsigned short int* max_codes;
    unsigned char* lengths;
    unsigned short int size;
    unsigned char max_bit_length;
	unsigned char is_fixed_hf;
} HFTable;

/* -------------------------------------------------------------------------------------------------------- */
// ------------------
//  Static Variables
// ------------------
// Fixed huffman literal/lengths codes
static const unsigned short int fixed_val_ptr[]          = {0, 256, 0, 280, 144};
static const unsigned short int fixed_mins[]             = {0, 0x00, 0x30, 0xC0, 0x190};
static const unsigned short int fixed_maxs[]             = {0, 0x18, 0xC0, 0xC8, 0x200};
static const unsigned short int fixed_distance_val_ptr[] = {0, 0x00};
static const unsigned short int fixed_distance_mins[]    = {0, 0x00};
static const unsigned short int fixed_distance_maxs[]    = {0, 0x20};

#define FIXED_LITERALS_HF(hf) 													\
	unsigned short int* literals_val_ptr = (unsigned short int*) fixed_val_ptr; \
	(hf) = (HFTable) {	 														\
		.values = &literals_val_ptr, 											\
		.min_codes = (unsigned short int*) fixed_mins, 							\
		.max_codes = (unsigned short int*) fixed_maxs, 							\
		.size = HF_LITERALS_SIZE, 												\
		.max_bit_length = 4, 													\
		.is_fixed_hf = IS_FIXED_LITERALS 										\
	}

#define FIXED_DISTANCE_HF(hf) 																\
	unsigned short int* distance_val_ptr = (unsigned short int*) fixed_distance_val_ptr; 	\
	(hf) = (HFTable) { 																		\
		.values = &distance_val_ptr, 														\
		.min_codes = (unsigned short int*) fixed_distance_mins, 							\
		.max_codes = (unsigned short int*) fixed_distance_maxs, 							\
		.size = HF_DISTANCE_SIZE, 															\
		.max_bit_length = 1, 																\
		.is_fixed_hf = TRUE 																\
	} 																						
	
/* ---------------------------------------------------------------------------------------------------------- */
// ------------------------
//  Functions Declarations
// ------------------------
static void deallocate_hf_table(HFTable* hf);
static int generate_codes(HFTable* hf);
static int decode_hf(BitStream* bit_stream, unsigned short int code, HFTable hf);
static int decode_lengths(BitStream* bit_stream, HFTable decoder_hf, HFTable* literals_hf, HFTable* distance_hf);
static int copy_data(unsigned char** dest, unsigned int* index, unsigned short int length, unsigned short int distance);
static int get_length(BitStream* bit_stream, unsigned short int value);
static int get_distance(BitStream* bit_stream, unsigned short int value);
static int read_uncompressed_data(BitStream* bit_stream, unsigned char** decompressed_data, unsigned int* decompressed_data_length);
static int decode_dynamic_huffman_tables(BitStream* bit_stream, HFTable* literals_hf, HFTable* distance_hf);

/// NOTE: the stream will be always deallocated both in case of failure and success.
/// 	  Furthermore, the function allocates the returned stream of bytes, so that
/// 	  once it's on the hand of the caller, it's responsible to manage that memory.
unsigned char* zlib_inflate(unsigned char* stream, unsigned int size, unsigned int* decompressed_data_length, int* zlib_err);

/* ---------------------------------------------------------------------------------------------------------- */

static void deallocate_hf_table(HFTable* hf) {
	if (hf -> is_fixed_hf) return;
	for (unsigned char i = 1; i <= hf -> max_bit_length; ++i) XCOMP_SAFE_FREE((hf -> values)[i]);
    XCOMP_SAFE_FREE(hf -> values);
    XCOMP_SAFE_FREE(hf -> min_codes);
    XCOMP_SAFE_FREE(hf -> max_codes);
    XCOMP_SAFE_FREE(hf -> lengths);
    hf -> max_bit_length = 0;
    hf -> size = 0;
    return;
}

static int generate_codes(HFTable* hf) {
	if (hf -> max_bit_length == 0) {
		hf -> max_bit_length = MAX_VALUE(hf -> max_bit_length, hf -> lengths, hf -> size);
	}

	unsigned char bl_count[16] = {0};
	for (unsigned short int i = 0; i < hf -> size; ++i) bl_count[(hf -> lengths)[i]]++;
    
	bl_count[0] = 0;
    hf -> values = (unsigned short int**) xcomp_calloc(hf -> max_bit_length + 1, sizeof(unsigned short int*));
    if (hf -> values == NULL) {
		WARNING_LOG("Failed to allocate buffer for hf -> values.\n");
		return -ZLIB_IO_ERROR;
	}
	
	for (unsigned char i = 1; i <= hf -> max_bit_length; ++i) {
        (hf -> values)[i] = (unsigned short int*) xcomp_calloc(bl_count[i], sizeof(unsigned short int));
		if ((hf -> values)[i] == NULL) {
			WARNING_LOG("Failed to allocate buffer for hf -> values[%u].\n", i);
			return -ZLIB_IO_ERROR;
		}
	}

	// Find the minimum and maximum code values for each bit_length 
    hf -> min_codes = (unsigned short int*) xcomp_calloc(hf -> max_bit_length + 1, sizeof(unsigned short int));
    if (hf -> min_codes == NULL) {
		WARNING_LOG("Failed to allocate buffer for hf -> min_codes.\n");
		return -ZLIB_IO_ERROR;
	}
	
	hf -> max_codes = (unsigned short int*) xcomp_calloc(hf -> max_bit_length + 1, sizeof(unsigned short int));
    if (hf -> max_codes == NULL) {
		WARNING_LOG("Failed to allocate buffer for hf -> max_codes.\n");
		return -ZLIB_IO_ERROR;
	}

    for (unsigned char bits = 1; bits <= hf -> max_bit_length; ++bits) {
        (hf -> min_codes)[bits] = ((hf -> min_codes)[bits - 1] + bl_count[bits - 1]) << 1;
        (hf -> max_codes)[bits] = (hf -> min_codes)[bits];
    }
	
	// Compute the code for each entry, and updating the max_code each time accordingly.
    // In this way we sort of allocate a code for each entry in order (as lexicographical order is required), as we already know the base for each bit_length (min_code).
	unsigned char* values_index = (unsigned char*) xcomp_calloc(hf -> max_bit_length + 1, sizeof(unsigned char));
    if (values_index == NULL) {
		WARNING_LOG("Failed to allocate buffer for values_index.\n");
		return -ZLIB_IO_ERROR;
	}
	
	for (unsigned short int i = 0; i < hf -> size; ++i) {
        if ((hf -> lengths)[i] != 0) {
            unsigned char value_bit_len = (hf -> lengths)[i];
            ((hf -> max_codes)[value_bit_len])++;
            (hf -> values)[value_bit_len][values_index[value_bit_len]++] = i;
		}
    }

    XCOMP_SAFE_FREE(values_index);

    return ZLIB_NO_ERROR;
}

static int decode_hf(BitStream* bit_stream, unsigned short int code, HFTable hf) {
	if (hf.is_fixed_hf) {
		// Read the first n common first bits, if bit_length is 4 means that is the literals hf so the base is 6 common bits, otherwise is 4 for distance hf
		for (unsigned char i = 0; i < (hf.max_bit_length == 4 ? 6 : 4); ++i) {
			unsigned char next_bit = SAFE_NEXT_BIT_READ(bit_stream, next_bit);
			code = (code << 1) + next_bit; 
		}
	}
	
	for (unsigned char i = 1; i <= hf.max_bit_length; ++i) {
		// Check if the code is inside this bit_length group, by checking if it's less than the maximum code for this bit_length.
		if (hf.max_codes[i] > code) { 
			// Return the corresponding decoded value from the hf tree, subtracting the min code for this bit length to get the offset of the entry.
			return (hf.is_fixed_hf) ? (code - hf.min_codes[i] + hf.values[0][i]) : hf.values[i][code - hf.min_codes[i]]; 
        }
		
		// If the code does not match, then add a bit and check with the next bit_length
        // If the hf used is the literals fixed one, then we need to do the following as the 8 bit-length group is divided in two sub-groups (0 - 143) and (280 - 287), preventing it to read the next_bit and skip the length
        if (i != 2 || hf.is_fixed_hf != IS_FIXED_LITERALS) { 
			unsigned char next_bit = SAFE_NEXT_BIT_READ(bit_stream, next_bit);
			code = (code << 1) + next_bit;
		}

        if (bit_stream -> error) break;
    }
	
	// Return max value to signal error during operation.
    return -ZLIB_INVALID_DECODED_VALUE; 
}

static int decode_lengths(BitStream* bit_stream, HFTable decoder_hf, HFTable* literals_hf, HFTable* distance_hf) {
    literals_hf -> lengths = (unsigned char*) xcomp_calloc(literals_hf -> size, sizeof(unsigned char));
	if (literals_hf -> lengths == NULL) {
		WARNING_LOG("Failed to allocate buffer for literals_hf -> lengths.\n");
		return -ZLIB_IO_ERROR;
	}
	
	distance_hf -> lengths = (unsigned char*) xcomp_calloc(distance_hf -> size, sizeof(unsigned char));
    if (distance_hf -> lengths == NULL) {
		XCOMP_SAFE_FREE(literals_hf -> lengths);
		WARNING_LOG("Failed to allocate buffer for distance_hf -> lengths.\n");
		return -ZLIB_IO_ERROR;
	}
	
	unsigned short int index = 0;
    while (index < (literals_hf -> size + distance_hf -> size)) {
		unsigned short int code = SAFE_NEXT_BIT_READ(bit_stream, code);
        int value = decode_hf(bit_stream, code, decoder_hf);

		if (value < 0) {
			XCOMP_MULTI_FREE(literals_hf -> lengths, distance_hf -> lengths);
			WARNING_LOG("Corrupted encoded lengths.\n");
			return value;
		}

		// NOTE: the operation index - literals_hf -> size, is required to normalize the index, as the values are read sequentially
        if (value < 16) {
			// 0 - 15: Represent code lengths of 0 - 15.
			// Hence, we first fill the literals and then the distances (look block order below)
            if (index < literals_hf -> size) {
				(literals_hf -> lengths)[index] = value; 
				literals_hf -> max_bit_length = MAX(literals_hf -> max_bit_length, value);
			} else {
				(distance_hf -> lengths)[index - literals_hf -> size] = value;
				distance_hf -> max_bit_length = MAX(distance_hf -> max_bit_length, value);
			}
			index++;
        } else if (value == 16) {
			// 16: Copy the previous code length 3 - 6 times.
			// Determine how many times to repeat
			unsigned char count = SAFE_BITS_READ(bit_stream, count, 2); 
            count += 3; 
			// Fill the literals and then the distances (look block order below)
            unsigned char value = (index < literals_hf -> size) ? (literals_hf -> lengths)[index - 1] : (distance_hf -> lengths)[index - literals_hf -> size - 1];
            for (unsigned char i = 0; i < count; ++i, ++index) {
                if (index < literals_hf -> size) (literals_hf -> lengths)[index] = value;
                else (distance_hf -> lengths)[index - literals_hf -> size] = value;
            }
        } else if (value == 17) {
			// 17: Repeat a code length of 0 for 3 - 10 times. (3 bits of length).
			unsigned char count = SAFE_BITS_READ(bit_stream, count, 3);
            count += 3; 
			for (unsigned char i = 0; i < count; ++i, ++index) {
                if (index < literals_hf -> size) (literals_hf -> lengths)[index] = 0;
                else (distance_hf -> lengths)[index - literals_hf -> size] = 0;
            }
        } else if (value == 18) {
			// 18: Repeat a code length of 0 for 11 - 138 times (7 bits of length).
			unsigned char count = SAFE_BITS_READ(bit_stream, count, 7);
            count += 11; 
			for (unsigned char i = 0; i < count; ++i, ++index) {
                if (index < literals_hf -> size) (literals_hf -> lengths)[index] = 0;
                else (distance_hf -> lengths)[index - literals_hf -> size] = 0;
            }
        } else {
			WARNING_LOG("Invalid value decoded: %u must be less than 19.\n", value);\
			return -ZLIB_CORRUPTED_DATA;
		}
    }

    return ZLIB_NO_ERROR;
}

/// Move backwards distance bytes in the output stream, and copy length bytes from this position to the output stream
static int copy_data(unsigned char** dest, unsigned int* index, unsigned short int length, unsigned short int distance) {
    *dest = (unsigned char*) xcomp_realloc(*dest, sizeof(unsigned char) * ((*index) + length));
	if (*dest == NULL) {
		WARNING_LOG("Failed to xcomp_reallocate buffer for dest.\n");
		return -ZLIB_IO_ERROR;
	}

	if ((long int) *index - distance < 0) {
		WARNING_LOG("Invalid distance, which makes buffer pointer negative: %d, (index: %u, distance: %u)\n", *index - distance, *index, distance);
		return -ZLIB_CORRUPTED_DATA;
	}

	mem_cpy(*dest + *index, *dest + *index - distance, sizeof(unsigned char) * length);
	(*index) += length;
	
	return ZLIB_NO_ERROR;
}

/// Get length from table defined in the specification.
static int get_length(BitStream* bit_stream, unsigned short int value) {
    const unsigned short int base_values[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    const unsigned char extra_bits[] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
		
    unsigned short int length = base_values[value - 257]; // 257 is the base value of the table, so it's used to normalize the value.
	unsigned char extra = SAFE_BITS_READ(bit_stream, extra, extra_bits[value - 257]); // Some of the entries require to read additional bits.

    return (length + extra);
}

/// Similarly, as above, we perform an analog lookup operation.
static int get_distance(BitStream* bit_stream, unsigned short int value) {
    const unsigned short int base_values[] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
    const unsigned char extra_bits[] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

    unsigned short int distance = base_values[value]; // The distance values start from 0 in the table.
    unsigned short int extra = SAFE_BITS_READ(bit_stream, extra, extra_bits[value]); // Similarly some entries require additional bits to be read.

    return (distance + extra);
}

static int read_uncompressed_data(BitStream* bit_stream, unsigned char** decompressed_data, unsigned int* decompressed_data_length) {
    // Read the length and its one-complement, and check if there's corruption
	skip_to_next_byte(bit_stream);
	unsigned short int length = SAFE_BYTE_READ_WITH_CAST(bit_stream, sizeof(unsigned short int), 1, unsigned short int, length, 0);
	unsigned short int length_c = SAFE_BYTE_READ_WITH_CAST(bit_stream, sizeof(unsigned short int), 1, unsigned short int, length_c, 0);
	XCOMP_BE_CONVERT(&length, sizeof(unsigned short int));
    XCOMP_BE_CONVERT(&length_c, sizeof(unsigned short int));
	unsigned short int check = ((length ^ length_c) + 1) & 0xFFFF;

    if (check) {
		WARNING_LOG("Invalid checksum: ((0x%X ^ 0x%X) + 1 = 0x%X) which is not equal to 0.\n", length, length_c, check);
		return -ZLIB_INVALID_LEN_CHECKSUM;
	} else if (length + bit_stream -> byte_pos > bit_stream -> size) {
		WARNING_LOG("Invalid length: %u would make bitstream pointer read out of bound.\n", length);
		PRINT_BIT_STREAM_INFO(bit_stream);
		return -ZLIB_CORRUPTED_DATA;
	}

	// Read length bytes from the stream
    *decompressed_data = (unsigned char*) xcomp_realloc(*decompressed_data, sizeof(unsigned char) * (*decompressed_data_length + length));
	if (mem_cpy(*decompressed_data + *decompressed_data_length, bitstream_read_bytes(bit_stream, sizeof(unsigned char), length), sizeof(unsigned char) * length) == NULL) {
		WARNING_LOG("Failed to xcomp_reallocate buffer for decompressed data.\n");
		return -ZLIB_IO_ERROR;
	}

	XCOMP_BE_CONVERT(*decompressed_data + *decompressed_data_length, length);
    *decompressed_data_length += length;
    
	return ZLIB_NO_ERROR;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * The format of the block:
 *  5 Bits: HLIT, # of Literal/Length codes - 257 (257 - 286)
 *  5 Bits: HDIST, # of Distance codes - 1        (1 - 32)
 *  4 Bits: HCLEN, # of Code Length codes - 4     (4 - 19)
 *
 * (HCLEN + 4) x 3 bits: code lengths for the code length
 * alphabet given just above, in the order: 16, 17, 18,
 *  0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
 *
 * These code lengths are interpreted as 3-bit integers
 * (0-7); as above, a code length of 0 means the
 * corresponding symbol (literal/length or distance code
 * length) is not used.
 *
 * HLIT + 257 code lengths for the literal/length alphabet,
 * encoded using the code length Huffman code
 *
 * HDIST + 1 code lengths for the distance alphabet,
 * encoded using the code length Huffman code
 *
 * The actual compressed data of the block, encoded using the literal/length and distance Huffman codes
 *
 * The literal/length symbol 256 (end of data), encoded using the literal/length Huffman code
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static int decode_dynamic_huffman_tables(BitStream* bit_stream, HFTable* literals_hf, HFTable* distance_hf) {
	HFTable decoder_hf = (HFTable) {0}; // Huffman Tree used for decoding the Literals and Distances Huffman Trees
	literals_hf -> size = SAFE_BITS_READ(bit_stream, literals_hf -> size, 5);
    literals_hf -> size += 257;
	distance_hf -> size = SAFE_BITS_READ(bit_stream, distance_hf -> size, 5);
    distance_hf -> size += 1;
	decoder_hf.size = SAFE_BITS_READ(bit_stream, decoder_hf.size, 4);
    decoder_hf.size += 4;

    // Retrieve the length to build the huffman tree to decode the other two huffman trees (Literals and Distance)
    const unsigned char order_of_code_lengths[] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
    decoder_hf.lengths = (unsigned char*) xcomp_calloc(HF_TABLE_SIZE, sizeof(unsigned char));
	if (decoder_hf.lengths == NULL) {
		WARNING_LOG("Failed to allocate buffer for decoder_hf lengths.\n");
		return -ZLIB_IO_ERROR;
	}

	// Retrieve the length of each code, using the array to match the fixed order of the codes. Furthermore, each one of the length is 3-bit long.
    for (unsigned char i = 0; i < decoder_hf.size; ++i) {
		(decoder_hf.lengths)[order_of_code_lengths[i]] = SAFE_BITS_READ(bit_stream, (decoder_hf.lengths)[order_of_code_lengths[i]], 3);
	}
    
	decoder_hf.size = HF_TABLE_SIZE; // The real size of the lengths is 19 as the amount allocated, value used for allocation
    
	// Build the huffman tree from the lengths
    int err = 0;
    if ((err = generate_codes(&decoder_hf)) < 0) {
		deallocate_hf_table(&decoder_hf);
		WARNING_LOG("An error occurred while generating the codes for the decoder_hf table.\n");
		return err;
	}

	// Decode the bit_lengths for both the Huffman Trees
	if ((err = decode_lengths(bit_stream, decoder_hf, literals_hf, distance_hf)) < 0) {
		deallocate_hf_table(&decoder_hf);
		WARNING_LOG("An error occurred while decoding the lengths.\n");
		return err;
	}

    deallocate_hf_table(&decoder_hf);

	// Again this values are required to be set as they are used for the allocation
    if (literals_hf -> size < HF_LITERALS_SIZE) {
		literals_hf -> lengths = xcomp_realloc(literals_hf -> lengths, sizeof(unsigned char) * HF_LITERALS_SIZE);
		if (literals_hf -> lengths == NULL) {
			XCOMP_SAFE_FREE(distance_hf -> lengths);
			WARNING_LOG("Failed to xcomp_reallocate the buffer for literals_hf -> lenghts.\n");
			return -ZLIB_IO_ERROR;
		}

		mem_set(literals_hf -> lengths + literals_hf -> size, 0, sizeof(unsigned char) * (HF_LITERALS_SIZE - literals_hf -> size));
		literals_hf -> size = HF_LITERALS_SIZE;
	}
	
	if (distance_hf -> size < HF_DISTANCE_SIZE) {
		distance_hf -> lengths = xcomp_realloc(distance_hf -> lengths, sizeof(unsigned char) * HF_DISTANCE_SIZE);
		if (distance_hf -> lengths == NULL) {
			XCOMP_SAFE_FREE(literals_hf -> lengths);
			WARNING_LOG("Failed to xcomp_reallocate the buffer for distance_hf -> lenghts.\n");
			return -ZLIB_IO_ERROR;
		}

		mem_set(distance_hf -> lengths + distance_hf -> size, 0, sizeof(unsigned char) * (HF_DISTANCE_SIZE - distance_hf -> size));
		distance_hf -> size = HF_DISTANCE_SIZE;
	}

    if ((err = generate_codes(literals_hf)) < 0) {
		XCOMP_SAFE_FREE(distance_hf -> lengths);
		deallocate_hf_table(literals_hf);
		WARNING_LOG("An error occurred while generating the codes for the literals_hf table.\n");
		return err;
	}
	
	XCOMP_SAFE_FREE(literals_hf -> lengths);

	if ((err = generate_codes(distance_hf)) < 0) {
		DEALLOCATE_TABLES(literals_hf, distance_hf);
		WARNING_LOG("An error occurred while generating the codes for the distance_hf table.\n");
		return err;
	}

	XCOMP_SAFE_FREE(distance_hf -> lengths);

    return ZLIB_NO_ERROR;
}

static int decode_compressed_block(BType compression_method, BitStream* bit_stream, unsigned char** decompressed_data, unsigned int* decompressed_data_length) {
	int err = 0;
	HFTable literals_hf = (HFTable) {0};
	HFTable distance_hf = (HFTable) {0};
	if (compression_method == COMPRESSED_FIXED_HF) {
		FIXED_LITERALS_HF(literals_hf);
		FIXED_DISTANCE_HF(distance_hf);
	} else if ((err = decode_dynamic_huffman_tables(bit_stream, &literals_hf, &distance_hf))) {
		WARNING_LOG("An error occurred during dynamic HF table decoding.\n");
		return err;
	}
	
	// Decode compressed data block
	unsigned short int code = 0;
	int decoded_value = 0;
	int decoded_distance = 0;
	while (!bit_stream -> error) {
		code = bitstream_read_next_bit(bit_stream);
		BITSTREAM_SAFE_CHECK(bit_stream, &literals_hf, &distance_hf);
		
		// Decode the literal/length value
		if ((decoded_value = decode_hf(bit_stream, code, literals_hf)) < 0) {
			DEALLOCATE_TABLES(&literals_hf, &distance_hf);
			WARNING_LOG("An error occurred while decoding the code.\n");
			return decoded_value;
		}
		
		if (decoded_value < 256) {
			// If literal/length value < 256: copy value (literal/length byte) to output stream
			*decompressed_data = (unsigned char*) xcomp_realloc(*decompressed_data, sizeof(unsigned char) * (*decompressed_data_length + 1));
			if (*decompressed_data == NULL) {
				DEALLOCATE_TABLES(&literals_hf, &distance_hf);
				WARNING_LOG("Failed to xcomp_reallocate buffer for decompressed data.\n");
				return -ZLIB_IO_ERROR;
			}
			
			(*decompressed_data)[*decompressed_data_length] = decoded_value;
			(*decompressed_data_length)++;
		} else if (decoded_value == 256) {
			break;
		} else {
			unsigned short int length = get_length(bit_stream, decoded_value);
			int distance_code = bitstream_read_next_bit(bit_stream);
			BITSTREAM_SAFE_CHECK(bit_stream, &literals_hf, &distance_hf);
			
			// Decode the distance value
			if ((decoded_distance = decode_hf(bit_stream, distance_code, distance_hf)) < 0) {
				DEALLOCATE_TABLES(&literals_hf, &distance_hf);
				WARNING_LOG("An error occurred while decoding the code.\n");
				return decoded_value;
			}
			
			// Move backwards distance bytes in the output stream, and copy length bytes from this position to the output stream
			unsigned short int distance = get_distance(bit_stream, decoded_distance);
			if ((err = copy_data(decompressed_data, decompressed_data_length, length, distance)) < 0) {
				DEALLOCATE_TABLES(&literals_hf, &distance_hf);
				WARNING_LOG("An error occurred while copying data from the decoded data.\n");
				return err;
			}
		}
	}
	
	DEALLOCATE_TABLES(&literals_hf, &distance_hf);
		
	return ZLIB_NO_ERROR;
}

unsigned char* zlib_inflate(unsigned char* stream, unsigned int size, unsigned int* decompressed_data_length, int* zlib_err) {
    // Initialize decompressed data
    unsigned char* decompressed_data = (unsigned char*) xcomp_calloc(1, sizeof(unsigned char));
    if (decompressed_data == NULL) {
		XCOMP_SAFE_FREE(stream);
		*zlib_err = -ZLIB_IO_ERROR;
		return ((unsigned char*) "Failed to allocate buffer for decompressed_data.\n");
	}

	BitStream bit_stream = CREATE_BIT_STREAM(stream, size);
	*decompressed_data_length = 0;

    unsigned char final = 0;
	BType compression_method = 0;
	unsigned int block_cnt = 0;
	unsigned int old_decompressed_size = 0;
	while (!final) {
		// Read header bits
		READ_BLOCK_HEADER(&bit_stream, *zlib_err, final, compression_method);
		block_cnt++;
		
		DEBUG_LOG("Block %u: is_final: %u, compression_method: '%s', ", block_cnt, final, btypes_str[compression_method]);
		
        if (compression_method == NO_COMPRESSION) {
			if ((*zlib_err = read_uncompressed_data(&bit_stream, &decompressed_data, decompressed_data_length)) < 0) {
				XCOMP_MULTI_FREE(stream, decompressed_data);
				return ((unsigned char*) "corrupted compressed block\n");
            }
            continue;
        } else if (compression_method == RESERVED) {
			XCOMP_MULTI_FREE(stream, decompressed_data);
            return ((unsigned char*) "invalid compression type\n");
        } 
		
        // Decode compressed data block
		if ((*zlib_err = decode_compressed_block(compression_method, &bit_stream, &decompressed_data, decompressed_data_length)) < 0) {
			XCOMP_MULTI_FREE(stream, decompressed_data);
			return ((unsigned char*) "An error occurred while decompressing the block.\n");
		}

		printf("decompressed_size: %u\n", *decompressed_data_length - old_decompressed_size);
		old_decompressed_size = *decompressed_data_length;
	}
		
	*zlib_err = ZLIB_NO_ERROR;
	XCOMP_SAFE_FREE(stream);

    return decompressed_data;
}

#endif //_ZLIB_DECOMPRESS_H_

