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
 *            zlib    <https://www.ietf.org/rfc/rfc1950.txt> *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// -------------------
//  Macros Definition
// -------------------
#define DEALLOCATE_TABLES(hf_a, hf_b)	\
	do {      							\
		deallocate_hf_table(hf_a);      \
		deallocate_hf_table(hf_b);      \
	} while (FALSE)      		

/* ---------------------------------------------------------------------------------------------------------- */
// ---------
//  Structs
// ---------
typedef struct HFTable {
	unsigned short int** values;
    unsigned short int* min_codes;
    unsigned short int* max_codes;
    unsigned char max_bit_length;
	unsigned char is_fixed_hf;
} HFTable;

typedef struct PACKED_STRUCT {
	unsigned char is_final:           1;
	unsigned char compression_method: 2;
	unsigned char padding:            5;
} ZLIBBlock;

typedef struct {
	unsigned char* data;
	unsigned int size;
	unsigned int pos;
} ZLIBBuffer; 

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

#define IS_FIXED_LITERALS 2
static void fixed_literals_hf(HFTable* hf) {
	unsigned short int* literals_val_ptr = (unsigned short int*) fixed_val_ptr; 
	*hf = (HFTable) {	 														
		.values = &literals_val_ptr, 											
		.min_codes = (unsigned short int*) fixed_mins, 							
		.max_codes = (unsigned short int*) fixed_maxs, 							
		.max_bit_length = 4, 													
		.is_fixed_hf = IS_FIXED_LITERALS 										
	};
	return;
}

static void fixed_distance_hf(HFTable* hf) {
	unsigned short int* distance_val_ptr = (unsigned short int*) fixed_distance_val_ptr; 	
	*hf = (HFTable) { 																		
		.values = &distance_val_ptr, 														
		.min_codes = (unsigned short int*) fixed_distance_mins, 							
		.max_codes = (unsigned short int*) fixed_distance_maxs, 							
		.max_bit_length = 1, 																
		.is_fixed_hf = TRUE 																
	}; 																						
	return;
}
	
/* ---------------------------------------------------------------------------------------------------------- */
// ------------------------
//  Functions Declarations
// ------------------------

/// NOTE: the stream will be always deallocated both in case of failure and success.
/// 	  Furthermore, the function allocates the returned stream of bytes, so that
/// 	  once it's on the hand of the caller, it's responsible to manage that memory.
unsigned char* deflate_inflate(unsigned char* stream, unsigned int size, unsigned int* decompressed_data_length, int* zlib_err);
unsigned char* zlib_inflate(unsigned char* stream, unsigned int size, unsigned int* decompressed_data_length, int* zlib_err);

/* ---------------------------------------------------------------------------------------------------------- */

static void deallocate_hf_table(HFTable* hf) {
	if (hf -> is_fixed_hf) return;
	for (unsigned int i = 1; i <= hf -> max_bit_length; ++i) XCOMP_SAFE_FREE((hf -> values)[i]);
    XCOMP_SAFE_FREE(hf -> values);
    XCOMP_SAFE_FREE(hf -> min_codes);
    XCOMP_SAFE_FREE(hf -> max_codes);
    return;
}

UNUSED_FUNCTION static void print_hf_table(HFTable* hf, unsigned char* size) {
	printf("\n-------------------\n");
	for (unsigned int i = 1; i <= hf -> max_bit_length; ++i) {
		if (size[i] == 0) continue;
		printf("%u: min_code: 0x%X, max_code: 0x%X, values: ", i, (hf -> min_codes)[i], (hf -> max_codes)[i]); 
		for (unsigned int j = 0; j < size[i]; ++j) {
			printf("0x%X ", (hf -> values)[i][j]);
		}
		printf("\n");
	}
	printf("\n-------------------\n");
}

static inline int max_value(unsigned char* arr, unsigned int size) {
	int max = 0;
	for (unsigned int i = 0; i < size; ++i) max = MAX(max, arr[i]);
	return max;
}

/// Generate huffman table (values, min_codes, max_codes) starting from given lengths
/// Should also be responsible for the eventual deallocation of the hf table
static int generate_hf(HFTable* hf, unsigned char* lengths, unsigned int size) {
	unsigned char bl_count[16] = {0};
	for (unsigned short int i = 0; i < size; ++i) (bl_count[lengths[i]])++;
    
	hf -> max_bit_length = max_value(lengths, size);
	hf -> values    = xcomp_calloc(hf -> max_bit_length + 1, sizeof(unsigned short int*));
    hf -> min_codes = xcomp_calloc(hf -> max_bit_length + 1, sizeof(unsigned short int));
	hf -> max_codes = xcomp_calloc(hf -> max_bit_length + 1, sizeof(unsigned short int));
    if ((hf -> min_codes == NULL) || (hf -> max_codes == NULL) || (hf -> values == NULL)) {
		WARNING_LOG("Failed to allocate buffers.");
		return -ZLIB_IO_ERROR;
	}
	
	for (unsigned int i = 1; i <= hf -> max_bit_length; ++i) {
        (hf -> values)[i] = (unsigned short int*) xcomp_calloc(bl_count[i], sizeof(unsigned short int));
		if ((hf -> values)[i] == NULL) {
			WARNING_LOG("Failed to allocate buffer for hf -> values[%u].", i);
			return -ZLIB_IO_ERROR;
		}
	}

	// Find the minimum and maximum code values for each bit_length 
	bl_count[0] = 0;
    for (unsigned int bits = 1; bits <= hf -> max_bit_length; ++bits) {
        (hf -> min_codes)[bits] = ((hf -> min_codes)[bits - 1] + bl_count[bits - 1]) << 1;
        (hf -> max_codes)[bits] = (hf -> min_codes)[bits];
    }
	
	// Compute the code for each entry, and update the max_code each time accordingly.
	// In this way we sort of allocate a code for each entry in order (as
	// lexicographical order is required), as we already know the base for each
	// bit_length (min_code).
	unsigned char* values_index = xcomp_calloc(hf -> max_bit_length + 1, sizeof(unsigned char));
    if (values_index == NULL) {
		WARNING_LOG("Failed to allocate buffer for values_index.");
		return -ZLIB_IO_ERROR;
	}
	
	for (unsigned int i = 0; i < size; ++i) {
        if (lengths[i] != 0) {
            unsigned char value_bit_len = lengths[i];
            ((hf -> max_codes)[value_bit_len])++;
            (hf -> values)[value_bit_len][values_index[value_bit_len]++] = i;
		}
    }

    XCOMP_SAFE_FREE(values_index);

	/* print_hf_table(hf, bl_count); */

    return ZLIB_NO_ERROR;
}

static int decode_hf(BitStream* bit_stream, unsigned short int code, HFTable hf, int* err) {
	if (hf.is_fixed_hf) {
		// Read the first n common first bits, if bit_length is 4 means that is
		// the literals hf so the base is 6 common bits, otherwise is 4 for
		// distance hf
		for (unsigned char i = 0; i < ((hf.max_bit_length == 4) ? 6 : 4); ++i) {
			code = (code << 1) + bitstream_read_next_bit(bit_stream);
			if (bit_stream -> error) {
				*err = -ZLIB_IO_ERROR;
				return *err;
			}
		}
	}
	
	for (unsigned char i = 1; i <= hf.max_bit_length; ++i) {
		/* DEBUG_LOG("%u: code: 0x%X", i, code); */
		// Check if the code is inside this bit_length group, by checking if
		// it's less than the maximum code for this bit_length.
		if (hf.max_codes[i] > code) { 
			// Return the corresponding decoded value from the hf tree,
			// subtracting the min code for this bit length to get the offset
			// of the entry.
			if (hf.is_fixed_hf) return (code - hf.min_codes[i] + (hf.values)[0][i]);
			else                return ((hf.values)[i][code - hf.min_codes[i]]); 
        }
		
		// If the code does not match, then add a bit and check with the next bit_length
		// If the hf used is the literals fixed one, then we need to do the
		// following as the 8 bit-length group is divided in two sub-groups (0 - 143) 
		// and (280 - 287), preventing it to read the next_bit and skip
		// the length
        if (i != 2 || hf.is_fixed_hf != IS_FIXED_LITERALS) { 
			code = (code << 1) + bitstream_read_next_bit(bit_stream);
		}

        if (bit_stream -> error) break;
    }
	
	// Return max value to signal error during operation.
	*err = -ZLIB_INVALID_DECODED_VALUE; 
    return *err;
}

static int decode_dhf_lengths(BitStream* bit_stream, HFTable decoder_hf, unsigned char* lengths, unsigned int size) {
	int err = 0;
    unsigned int i = 0; 
	while (i < size) {
        int value = decode_hf(bit_stream, bitstream_read_next_bit(bit_stream), decoder_hf, &err);
		if (value < 0 || value > 18) {
			WARNING_LOG("Corrupted encoded lengths.");
			break;
		}

		// 0 - 15: Represent code lengths of 0 - 15.
        if (value < 16) {
			lengths[i++] = value;
			continue;
        } 
		
		const unsigned char bit_sizes[] = { 2, 3, 7 };
		const unsigned char cnt_base[]  = { 3, 3, 11 };
		unsigned char count = 0;
		bitstream_read_bits(bit_stream, bit_sizes[value - 16], &count);
		count += cnt_base[value - 16];
		if (bit_stream -> error) {
			err = -ZLIB_IO_ERROR;
			break;
		}

		// 16: Copy the previous code length 3 - 6 times.
		// 17: Repeat a code length of 0 for 3 - 10 times. (3 bits of length).
		// 18: Repeat a code length of 0 for 11 - 138 times (7 bits of length).
		const unsigned char copy_value = (value == 16) * lengths[i - 1];
		for (unsigned int idx = 0; idx < count; ++i, ++idx) lengths[i] = copy_value;
	}
	
	if (err < 0) {
		deallocate_hf_table(&decoder_hf);
		XCOMP_SAFE_FREE(lengths);
	}

    return err;
}

// Parse the dynamic huffman table header 
static int parse_decoder_hf(BitStream* bit_stream, HFTable* decoder_hf, dhf_header_t* dhf_header) {
	bitstream_read_bits(bit_stream, 5, &(dhf_header -> hlit));
	bitstream_read_bits(bit_stream, 5, &(dhf_header -> hdist));
	bitstream_read_bits(bit_stream, 4, &(dhf_header -> hclen));
    if (bit_stream -> error) return -ZLIB_IO_ERROR;
	
	dhf_header -> hlit  += 257;
	dhf_header -> hdist += 1;
	dhf_header -> hclen += 4;

	DEBUG_LOG("hlit: %u, hdist: %u, hclen: %u", dhf_header -> hlit, dhf_header -> hdist, dhf_header -> hclen);

    // Retrieve the length to build the huffman tree to decode the other two huffman trees (Literals and Distance)
    const unsigned char order_of_code_lengths[] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
    unsigned char* lengths = xcomp_calloc(HF_TABLE_SIZE, sizeof(unsigned char));
	if (lengths == NULL) {
		WARNING_LOG("Failed to allocate buffer for decoder_hf lengths.");
		return -ZLIB_IO_ERROR;
	}

	// Retrieve the length of each code, using the array to match the fixed
	// order of the codes. Furthermore, each one of the length is 3-bit long.
    for (unsigned char i = 0; i < dhf_header -> hclen; ++i) {
		bitstream_read_bits(bit_stream, 3, lengths + order_of_code_lengths[i]);
		if (bit_stream -> error) {
			XCOMP_SAFE_FREE(lengths);
			return -ZLIB_IO_ERROR;
		}
	}
    
	int err = 0;
    if ((err = generate_hf(decoder_hf, lengths, HF_TABLE_SIZE)) < 0) {
		WARNING_LOG("An error occurred while generating the codes for the decoder_hf table.");
		return err;
	}
	
	XCOMP_SAFE_FREE(lengths);

	return ZLIB_NO_ERROR;
}

/// Decode the Literal and Distance Dynamic Huffman Tables
static int decode_dhf_tables(BitStream* bit_stream, HFTable* literals_hf, HFTable* distance_hf) {
    int err = 0;
	dhf_header_t dhf_header = {0};
	HFTable decoder_hf = {0}; 
    if ((err = parse_decoder_hf(bit_stream, &decoder_hf, &dhf_header)) < 0) {
		WARNING_LOG("An error occurred while generating the codes for the decoder_hf table.");
		return err;
	}
	
    unsigned char* hf_lengths   = (unsigned char*) xcomp_calloc(dhf_header.hlit + dhf_header.hdist, sizeof(unsigned char));
    unsigned char* lit_lengths  = (unsigned char*) xcomp_calloc(HF_LITERALS_SIZE, sizeof(unsigned char));
	unsigned char* dist_lengths = (unsigned char*) xcomp_calloc(HF_DISTANCE_SIZE, sizeof(unsigned char));
	if ((hf_lengths == NULL) || (lit_lengths == NULL) || (dist_lengths == NULL)) {
		deallocate_hf_table(&decoder_hf);
		XCOMP_MULTI_FREE(hf_lengths, lit_lengths, dist_lengths);
		WARNING_LOG("Failed to allocate buffer for distance_hf -> lengths.");
		return -ZLIB_IO_ERROR;
	}

	// Decode the bit_lengths for both the Huffman Trees
	if ((err = decode_dhf_lengths(bit_stream, decoder_hf, hf_lengths, dhf_header.hlit + dhf_header.hdist)) < 0) {
		XCOMP_MULTI_FREE(lit_lengths, dist_lengths);
		WARNING_LOG("An error occurred while decoding the literals lengths.");
		return err;
	}

	mem_cpy(lit_lengths, hf_lengths, dhf_header.hlit);
	mem_cpy(dist_lengths, hf_lengths + dhf_header.hlit, dhf_header.hdist);
	deallocate_hf_table(&decoder_hf);
	XCOMP_SAFE_FREE(hf_lengths);

	err = generate_hf(literals_hf, lit_lengths, HF_LITERALS_SIZE);
	if (err == 0) err = generate_hf(distance_hf, dist_lengths, HF_DISTANCE_SIZE);

	if (err < 0) {
		WARNING_LOG("An error occurred while generating literal and distance dhfs.");
		DEALLOCATE_TABLES(literals_hf, distance_hf);
	}
	
	XCOMP_MULTI_FREE(lit_lengths, dist_lengths);

    return err;
}

/// Move backwards distance bytes in the output stream, and copy length bytes from this position to the output stream
static int copy_data(ZLIBBuffer* buffer, unsigned short int length, unsigned short int distance) {
	if (buffer -> pos < distance) {
		WARNING_LOG("Invalid distance, which makes buffer pointer negative: %d, (index: %u, distance: %u)", buffer -> pos - distance, buffer -> pos, distance);
		return -ZLIB_CORRUPTED_DATA;
	}

	const int copy_len = MIN(MAX((buffer -> size - buffer -> pos), 0), length);
	if (copy_len > 0) {
		mem_cpy(buffer -> data + buffer -> pos, buffer -> data + buffer -> pos - distance, copy_len);
		buffer -> pos += copy_len;
	}
	
	return ZLIB_NO_ERROR;
}

/// Get length from table defined in the specification.
static int get_length(BitStream* bit_stream, unsigned short int value) {
    const unsigned short int base_values[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    const unsigned char extra_bits[]       = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
	
	// Some of the entries require to read additional bits.
	unsigned char extra = 0;
	bitstream_read_bits(bit_stream, extra_bits[value - 257], &extra);
	if (bit_stream -> error) return -ZLIB_IO_ERROR;

    return (base_values[value - 257] + extra);
}

/// Similarly, as above, we perform a lookup operation.
static int get_distance(BitStream* bit_stream, unsigned short int value) {
    const unsigned short int base_values[] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
    const unsigned char extra_bits[]       = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
	
	// Some of the entries require to read additional bits.
	int extra = 0;
	bitstream_read_bits(bit_stream, extra_bits[value], &extra);
	if (bit_stream -> error) return -ZLIB_IO_ERROR;
	
    return (base_values[value] + extra);
}


// TODO: Should probably handle the difference between fixed HF and dynamic HF
static int decode_compressed_block(BType compression_method, BitStream* bit_stream, ZLIBBuffer* buffer, int* zlib_err) { 
	HFTable literals_hf = (HFTable) {0};
	HFTable distance_hf = (HFTable) {0};
	if (compression_method == COMPRESSED_FIXED_HF) {
		fixed_literals_hf(&literals_hf);
		fixed_distance_hf(&distance_hf);
	} else if ((*zlib_err = decode_dhf_tables(bit_stream, &literals_hf, &distance_hf)) < 0) {
		WARNING_LOG("An error occurred during dynamic HF table decoding.");
		return *zlib_err;
	}

	while ((bit_stream -> error == 0) && (*zlib_err == 0)) {
		// Decode the literal/length value
		int literal = decode_hf(bit_stream, bitstream_read_next_bit(bit_stream), literals_hf, zlib_err);
		if (literal < 0) {
			DEBUG_LOG("Failed at decoding the literal");
			break;
		}
		
		if (literal == 256) break;
		else if (literal < 256) {
			// literal/length value < 256: copy value (literal/length byte) to output stream
			(buffer -> data)[(buffer -> pos)++] = literal;
		} else {
			int length = get_length(bit_stream, literal);
			
			// Decode the distance value
			int distance = decode_hf(bit_stream, bitstream_read_next_bit(bit_stream), distance_hf, zlib_err);
			if (distance < 0) break;
			distance = get_distance(bit_stream, distance);
		
			// Move backwards distance bytes in the output stream, and copy
			// length bytes from this position to the output stream
			*zlib_err = copy_data(buffer, length, distance);
		}
	}
	
	if (bit_stream -> error) *zlib_err = -ZLIB_IO_ERROR;	
	DEALLOCATE_TABLES(&literals_hf, &distance_hf);
	
	return *zlib_err;
}

static int read_uncompressed_data(BitStream* bit_stream, ZLIBBuffer* buffer) {
	skip_to_next_byte(bit_stream);
	
    // Read the length and its one-complement, and check if there's corruption
	unsigned short int length = 0;
	mem_cpy(&length, bitstream_read_bytes(bit_stream, sizeof(unsigned short int), 1), sizeof(unsigned short int));
	if (bit_stream -> error) return -ZLIB_IO_ERROR;

	unsigned short int length_c = 0;
	mem_cpy(&length_c, bitstream_read_bytes(bit_stream, sizeof(unsigned short int), 1), sizeof(unsigned short int));
	if (bit_stream -> error) return -ZLIB_IO_ERROR;

	const unsigned short int check = ((length ^ length_c) + 1) & 0xFFFF;
    if (check) {
		WARNING_LOG("Corrupted length: ((0x%X ^ 0x%X) + 1 = 0x%X) which is not equal to 0.", length, length_c, check);
		return -ZLIB_INVALID_LEN_CHECKSUM;
	} else if (length + bit_stream -> byte_pos > bit_stream -> size) {
		WARNING_LOG("Invalid length: %u would make bitstream pointer read out of bound.", length);
		PRINT_BIT_STREAM_INFO(bit_stream);
		return -ZLIB_CORRUPTED_DATA;
	}

	// Read length bytes from the stream
	const unsigned int copy_size = MIN(MAX((buffer -> size - buffer -> pos), 0), length);
	if (copy_size > 0) {
		mem_cpy(buffer -> data + buffer -> pos, bitstream_read_bytes(bit_stream, sizeof(unsigned char), length), copy_size);
		if (bit_stream -> error) return -ZLIB_IO_ERROR;
		buffer -> pos += copy_size;
	}

	return ZLIB_NO_ERROR;
}

static int read_block_header(BitStream* bit_stream, ZLIBBlock* block, int* zlib_err) {
	mem_set(block, 0, sizeof(ZLIBBlock));
	bitstream_read_bits(bit_stream, 3, block);
	if (bit_stream -> error) *zlib_err = -ZLIB_IO_ERROR;														
	return *zlib_err;
}

static unsigned char* deflate_block(BitStream* bit_stream, ZLIBBuffer* buffer, ZLIBBlock* block, int* zlib_err) {
	// Read header bits
	if ((read_block_header(bit_stream, block, zlib_err)) < 0) {
		*zlib_err = -ZLIB_IO_ERROR;
		return NULL;
	}

	DEBUG_LOG("%sBlock: compression_method: '%s'", block -> is_final ? "FINAL " : "", btypes_str[block -> compression_method]);
	
	if (block -> compression_method == RESERVED) {
		*zlib_err = -ZLIB_INVALID_COMPRESSION_METHOD; 
		return NULL; 
	} else if (block -> compression_method == NO_COMPRESSION) {
		*zlib_err = read_uncompressed_data(bit_stream, buffer);
		if (*zlib_err < 0) return NULL;
	} else {
		// Decode compressed data block
		*zlib_err = decode_compressed_block(block -> compression_method, bit_stream, buffer, zlib_err);
		if (*zlib_err < 0) return NULL;
	}

	return buffer -> data;
}

static unsigned char* zlib_raw_inflate(BitStream* bit_stream, unsigned int window_size, unsigned int* decompressed_data_length, int* zlib_err) {
	ZLIBBuffer buffer = { .pos = 0, .size = window_size };
	const unsigned int max_data_length = *decompressed_data_length;
	if (max_data_length > 0) buffer.size = max_data_length;
	
	buffer.data = (unsigned char*) xcomp_calloc(buffer.size, sizeof(unsigned char));
    if (buffer.data == NULL) {
		*zlib_err = -ZLIB_IO_ERROR;
		return NULL;
	}

    ZLIBBlock block = {0};
	while (!block.is_final) {
		if ((max_data_length > 0) && (buffer.pos > max_data_length)) break;
		
		deflate_block(bit_stream, &buffer, &block, zlib_err);
		if (*zlib_err < 0) {
			XCOMP_SAFE_FREE(buffer.data);
			return NULL;
		}

		if (max_data_length == 0) {
			buffer.size += window_size;
			buffer.data = realloc(buffer.data, buffer.size);
		    if (buffer.data == NULL) {
				*zlib_err = -ZLIB_IO_ERROR;
				return NULL;
			}
		}
	}
	
	*decompressed_data_length = buffer.pos;
	buffer.data = realloc(buffer.data, buffer.pos);
	if (buffer.data == NULL) {
		*zlib_err = -ZLIB_IO_ERROR;
		return NULL;
	}
	
	return buffer.data;
}

// -------------------------------------------------------------------------------------------
// Decode Raw DEFLATE compressed data
unsigned char* deflate_inflate(unsigned char* stream, unsigned int size, unsigned int* decompressed_data_length, int* zlib_err) {
	BitStream bit_stream = CREATE_BIT_STREAM(stream, size);
	unsigned char* decompressed_data = zlib_raw_inflate(&bit_stream, WINDOW_SIZE, decompressed_data_length, zlib_err);
    if (*zlib_err < 0) XCOMP_SAFE_FREE(stream);
	return decompressed_data;
}

// -------------------------------------------------------------------------------------------
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

static int read_zlib_header(BitStream* bit_stream, zlib_header_t* zlib_header) {
	unsigned char compress_data = bitstream_read_next_byte(bit_stream);
	if (bit_stream -> error) return -ZLIB_IO_ERROR;
    
	unsigned char flags = bitstream_read_next_byte(bit_stream);
	if (bit_stream -> error) return -ZLIB_IO_ERROR;

	zlib_header -> compression_method = compress_data & 0x0F;
    zlib_header -> window_size        = (compress_data >> 4) & 0x0F;
    zlib_header -> preset_dictionary  = ((flags & 0x20) >> 5) & 0x01;
    zlib_header -> compression_level  = ((flags & 0xC0) >> 6) & 0x03;

    if (zlib_header -> compression_method != 8)       return -ZLIB_INVALID_COMPRESSION_METHOD;
	else if (zlib_header -> window_size > 7)          return -ZLIB_INVALID_WINDOW_SIZE;
	else if (zlib_header -> preset_dictionary)        return -ZLIB_DICTIONARY_NOT_SUPPORTED;
	else if ((compress_data * 256 + flags) % 31 != 0) return -ZLIB_INVALID_CHECKSUM;
    
	zlib_header -> window_size = 1 << (zlib_header -> window_size + 8);

	DEBUG_LOG("-- ZLIB HEADER --");
    DEBUG_LOG("compression method: %u", zlib_header -> compression_method);
    DEBUG_LOG("window size:        %u", zlib_header -> window_size);
    DEBUG_LOG("preset dictionary:  %u", zlib_header -> preset_dictionary);
    DEBUG_LOG("compression level:  %u", zlib_header -> compression_level);
	DEBUG_LOG("-----------------");

    return 0;
}

unsigned char* zlib_inflate(unsigned char* stream, unsigned int size, unsigned int* decompressed_data_length, int* zlib_err) {
	BitStream bit_stream = CREATE_BIT_STREAM(stream, size);
	*decompressed_data_length = 0;
    
	zlib_header_t zlib_header = {0};
	*zlib_err = read_zlib_header(&bit_stream, &zlib_header);
    if (*zlib_err) return ((unsigned char*) "Invalid ZLIB Header");

	unsigned char* decompressed_data = zlib_raw_inflate(&bit_stream, zlib_header.window_size, decompressed_data_length, zlib_err);
	if (*zlib_err) {
    	XCOMP_SAFE_FREE(stream);
		return ((unsigned char*) "Failed to decompress data");
	}

    // Read the ADLER-CRC
    unsigned int adler_crc = 0;
	mem_cpy(&adler_crc, stream + size - 4, sizeof(unsigned int));
	XCOMP_BE_CONVERT(&adler_crc, sizeof(unsigned int));
	XCOMP_SAFE_FREE(stream);

    // Calculate the ADLER-CRC of the blocks
    DEBUG_LOG("decompressed_data_length: %u", *decompressed_data_length);
    
	unsigned int adler_register = __adler_crc(decompressed_data, *decompressed_data_length, 1);
    if (adler_crc != adler_register) {
        *zlib_err = -ZLIB_INVALID_ADLER_CHECKSUM;
        XCOMP_SAFE_FREE(decompressed_data);
        DEBUG_LOG("adler_register: 0x%X, adler_crc: 0x%X", adler_register, adler_crc);
        return ((unsigned char*) "corrupted compressed data blocks");
    }

	*zlib_err = ZLIB_NO_ERROR;
	return decompressed_data; 
}

#endif //_ZLIB_DECOMPRESS_H_

