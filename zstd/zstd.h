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

#ifndef _ZSTD_H_
#define _ZSTD_H_

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Resources: zstd <http://github.com/facebook/zstd> (original repo) *
 *                 <https://datatracker.ietf.org/doc/html/rfc8878>   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "./xxhash64.h"
#include "./bitstream.h"

// -----------------
//  Constant Values 
// -----------------
#define ZSTD_SKIPPABLE_FRAME_MAGIC_MIN 0x184D2A50
#define ZSTD_SKIPPABLE_FRAME_MAGIC_MAX 0x184D2A5F
#define ZSTD_FRAME_MAGIC               0xFD2FB528
#define FSE_TABLELOG_ABSOLUTE_MAX      15
#define FSE_MAX_SYMBOL_VALUE           255
#define MAXIMUM_CODE_LENGTH            11
#define NOT_USING_RLE                  -1
#define MAX_LL_CODE                    35
#define MAX_ML_CODE                    52
#define MAX_OL_CODE   	               31
#define U32_BITS                       32
#define MAX_BLOCK_SIZE                (128 * 1024)

/* -------------------------------------------------------------------------------------------------------- */
// -------------------
//  Macros Definition
// -------------------
#define GET_LITERALS_FIELDS_BIT_SIZE(size_format) ((size_format == 0 || size_format == 1) ? 10 : (size_format == 2 ? 14 : 18))
#define GET_FRAME_CONTENT_SIZE(size_flag, single_seg_flag) (size_flag == 0 ? single_seg_flag : 1 << size_flag) 
#define UPDATE_FSE_STATE(state, fse_table, compressed_bit_stream) state = (fse_table)[state].baseline + reversed_bitstream_read_bits(&compressed_bit_stream, (fse_table)[state].nb_bits)
#define UPDATE_HF_STATE(state, hf_literals, hf_table_size, reversed_bit_stream) state = ((state << (hf_literals)[state].nb_bits) & (hf_table_size - 1)) | reversed_bitstream_read_bits(reversed_bit_stream, (hf_literals)[state].nb_bits)
#define INFER_LAST_STREAM_SIZE(total_streams_size, streams_size) (total_streams_size - 6 - streams_size[0] - streams_size[1] - streams_size[2])
#define SKIP_PADDING(err, reversed_bit_stream) 																						\
	do { 																															\
		unsigned char bit_val = reversed_bitstream_read_next_bit(reversed_bit_stream); 												\
		if (((reversed_bit_stream) -> bit_pos == 6) && ((reversed_bit_stream) -> byte_pos < (reversed_bit_stream) -> size - 1)) {	\
			WARNING_LOG("Padding zeros cannot be more than 7.\n");																	\
			err = -ZSTD_CORRUPTED_DATA;																								\
			break;																													\
		} else if (bit_val) break; 																									\
	} while(TRUE)

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

typedef enum PACKED_STRUCT LiteralsBlockType { RAW_LITERALS_BLOCK, RLE_LITERALS_BLOCK, COMPRESSED_LITERALS_BLOCK, TREELESS_LITERALS_BLOCK } LiteralsBlockType;
typedef enum PACKED_STRUCT BlockType { RAW_BLOCK, RLE_BLOCK, COMPRESSED_BLOCK, RESERVED_TYPE } BlockType;
typedef enum PACKED_STRUCT CompressionMode { PREDEFINED_MODE, RLE_MODE, FSE_COMPRESSED_MODE, REPEAT_MODE } CompressionMode;

#ifdef _DEBUG
	static const char* literals_blocks_type_str[] = { "RAW_LITERALS_BLOCK", "RLE_LITERALS_BLOCK", "COMPRESSED_LITERALS_BLOCK", "TREELESS_LITERALS_BLOCK" };
	static const char* block_types_str[] = { "RAW_BLOCK", "RLE_BLOCK", "COMPRESSED_BLOCK", "RESERVED_TYPE" };
	static const char* compression_modes_str[] = { "PREDEFINED_MODE", "RLE_MODE", "FSE_COMPRESSED_MODE", "REPEAT_MODE" };
#endif //_DEBUG

/* -------------------------------------------------------------------------------------------------------- */
// -------------------------------
//  Predefined Distributions Data
// -------------------------------
#define LL_MAX_LOG        9
#define ML_MAX_LOG        9
#define OL_MAX_LOG        8
#define PRED_LL_TABLE_LOG 6
#define PRED_ML_TABLE_LOG 6
#define PRED_OL_TABLE_LOG 5

static const short int ll_pred_frequencies[] = { 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 1, 1, 1, 1, 1, -1, -1, -1, -1 };
static const short int ml_pred_frequencies[] = { 1, 4, 3, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1, -1 };
static const short int ol_pred_frequencies[] = { 1, 1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1 };

static const unsigned int ll_codes[36][2] = {
	{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0}, {10, 0}, {11, 0}, {12, 0}, {13, 0}, {14, 0}, {15, 0}, {16, 1}, {18, 1}, {20, 1}, {22, 1}, {24, 2}, {28, 2}, {32, 3}, {40, 3}, {48, 4}, {64, 6}, {128, 7}, {256, 8}, {512, 9}, {1024, 10}, {2048, 11}, {4096, 12}, {8192, 13}, {16384, 14}, {32768, 15}, {65536, 16}
};

static const unsigned int ml_codes[53][2] = {
	{3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0}, {10, 0}, {11, 0}, {12, 0}, {13, 0}, {14, 0}, {15, 0}, {16, 0}, {17, 0}, {18, 0}, {19, 0}, {20, 0}, {21, 0}, {22, 0}, {23, 0}, {24, 0}, {25, 0}, {26, 0}, {27, 0}, {28, 0}, {29, 0}, {30, 0}, {31, 0}, {32, 0}, {33, 0}, {34, 0}, {35, 1}, {37, 1}, {39, 1}, {41, 1}, {43, 2}, {47, 2}, {51, 3}, {59, 3}, {67, 4}, {83, 4}, {99, 5}, {131, 7}, {259, 8}, {515, 9}, {1027, 10}, {2051, 11}, {4099, 12}, {8195, 13}, {16387, 14}, {32771, 15}, {65539, 16}
};

/* -------------------------------------------------------------------------------------------------------- */
// ---------
//  Structs
// ---------
typedef struct PACKED_STRUCT LengthCode {
	unsigned int value;
	unsigned int num_bits;
} LengthCode;

typedef struct PACKED_STRUCT FrameHeaderDescriptor {
	unsigned char dictionary_id_flag: 2;
	unsigned char content_checksum_flag: 1;
	unsigned char reserved: 1;
	unsigned char unused: 1;
	unsigned char single_segment_flag: 1;
	unsigned char frame_content_size_flag: 2;
} FrameHeaderDescriptor;

typedef struct PACKED_STRUCT WindowDescriptor {
	unsigned char mantissa: 3;
	unsigned char exponent: 4;
} WindowDescriptor;

typedef struct PACKED_STRUCT BlockHeader {
	unsigned char last_block: 1;
	BlockType block_type: 2;
	unsigned int block_size: 21;
} BlockHeader;

typedef struct PACKED_STRUCT SymbolCompressionModes {
	unsigned char reserved: 2;
	CompressionMode match_len_mode: 2;
	CompressionMode offset_mode: 2;
	CompressionMode literals_len_mode: 2;
} SymbolCompressionModes;

typedef struct FSETableEntry {
	unsigned char symbol;   // Decoded symbol
    unsigned char nb_bits;  // Bits to read for next state
    unsigned short int baseline; // Base for next state calculation
} FSETableEntry;

typedef struct PACKED_STRUCT LiteralsSectionHeader {
	unsigned int regenerated_size;
	unsigned int compressed_size;
	LiteralsBlockType literals_block_type;
	unsigned char streams_cnt;
} LiteralsSectionHeader;

typedef struct PACKED_STRUCT ZSTDHfEntry {
	unsigned char symbol;
	unsigned char nb_bits;
} ZSTDHfEntry;

typedef struct SequenceSection {
	FSETableEntry* ll_fse_table;
	unsigned char ll_table_log;
	short int ll_rle;
	FSETableEntry* ml_fse_table;
	unsigned char ml_table_log;
	short int ml_rle; 
	FSETableEntry* ol_fse_table;
	unsigned char ol_table_log;
	short int ol_rle;
} SequenceSection;

typedef struct PACKED_STRUCT Sequence {
	unsigned int ll_value;
	unsigned int ml_value;
	unsigned int ol_value;
} Sequence;

typedef struct Workspace {
	unsigned int* offset_history;
	unsigned char* frame_buffer;
	unsigned int frame_buffer_len;
	SequenceSection sequence_section;
	ZSTDHfEntry* hf_literals;
	unsigned char table_log;
	unsigned char* literals; 
	unsigned int literals_cnt;
	Sequence* sequences;
	unsigned int sequence_len;
	unsigned int hf_tree_desc_size;
	unsigned char max_nb_bits;
} Workspace;

/* -------------------------------------------------------------------------------------------------------- */
// ------------------------
//  Functions Declarations
// ------------------------
static unsigned char highest_bit(unsigned long long int val);
static void print_fhd(FrameHeaderDescriptor fhd);
static void deallocate_workspace(Workspace* workspace);
static void deallocate_sequence_section(SequenceSection* sequence_section);
static int calc_baseline_and_numbits(unsigned int num_states_total, unsigned int num_states_symbol, unsigned int state_number, unsigned short int* a, unsigned char* b);
static int read_probabilities(BitStream* compressed_bit_stream, unsigned char table_log, unsigned char max_symbol, short int** frequencies, unsigned short int* probabilities_cnt);
static int fse_build_table(unsigned char table_log, short int* frequencies, unsigned short int probabilities_cnt, FSETableEntry** fse_table);
static int read_weights(BitStream* compressed_bit_stream, unsigned char* table_log, unsigned char** weights, unsigned short int* weights_cnt);
static int build_huff_table(BitStream* compressed_bit_stream, Workspace* workspace);
static int huff_decode_stream(BitStream* literals_stream, unsigned short int hf_literals_size, unsigned int regenerated_size, Workspace* workspace);
static int decode_literals(BitStream* compressed_bit_stream, Workspace* workspace, LiteralsSectionHeader lsh);
static int parse_literals_section(BitStream* compressed_bit_stream, Workspace* workspace);
static int decode_sequences(BitStream* sequence_compressed_bit_stream, Workspace* workspace);
static int parse_sequence_section(BitStream* compressed_bit_stream, Workspace* workspace);
static unsigned int update_off_history(unsigned int* offset_history, unsigned int offset, unsigned int ll_value);
static int sequence_execution(Workspace* workspace);
static int decompress_block(BitStream* compressed_bit_stream, Workspace* workspace);
static int parse_block(BitStream* bit_stream, Workspace* workspace, unsigned int block_maximum_size);
static int parse_frames(BitStream* bit_stream, unsigned char** decompressed_data, unsigned int* decompressed_data_length);

/// NOTE: the stream will be always deallocated both in case of failure and success.
/// 	  Furthermore, the function allocates the returned stream of bytes, so that
/// 	  once it's on the hand of the caller, it's responsible to manage that memory.
unsigned char* zstd_inflate(unsigned char* stream, unsigned int size, unsigned int* decompressed_data_length, int* zstd_err);

/* ---------------------------------------------------------------------------------------------------------- */

// ---------------------------
//  General Utilities Section
// ---------------------------

static unsigned char highest_bit(unsigned long long int val) {
	return U32_BITS - __builtin_clz(val);
}

static void print_fhd(FrameHeaderDescriptor fhd) {
	(void) fhd;
	DEBUG_LOG("FrameHeaderDescriptor: (0x%X)\n", *QCOW_CAST_PTR(&fhd, unsigned char));
    DEBUG_LOG(" - frame_content_size_flag: %u\n", fhd.frame_content_size_flag);
    DEBUG_LOG(" - single_segment_flag: %u\n", fhd.single_segment_flag);
    DEBUG_LOG(" - unused: %u\n", fhd.unused);
    DEBUG_LOG(" - reserved: %u\n", fhd.reserved);
    DEBUG_LOG(" - content_checksum_flag: %u\n", fhd.content_checksum_flag);
    DEBUG_LOG(" - dictionary_id_flag: %u\n", fhd.dictionary_id_flag);
	return;
}

static void deallocate_workspace(Workspace* workspace) {
	QCOW_SAFE_FREE(workspace -> frame_buffer);
	QCOW_SAFE_FREE(workspace -> sequence_section.ll_fse_table);
	QCOW_SAFE_FREE(workspace -> sequence_section.ml_fse_table);
	QCOW_SAFE_FREE(workspace -> sequence_section.ol_fse_table);
	QCOW_SAFE_FREE(workspace -> hf_literals);
	QCOW_SAFE_FREE(workspace -> literals); 
	QCOW_SAFE_FREE(workspace -> sequences);
	return;
}

static void deallocate_sequence_section(SequenceSection* sequence_section) {
	if (sequence_section -> ll_fse_table) QCOW_SAFE_FREE(sequence_section -> ll_fse_table);
	if (sequence_section -> ml_fse_table) QCOW_SAFE_FREE(sequence_section -> ml_fse_table);
	if (sequence_section -> ol_fse_table) QCOW_SAFE_FREE(sequence_section -> ol_fse_table);
	return;
}

// ---------------------------------------
//  Literals Parsing and Decoding Section
// ---------------------------------------
// TODO: revise a simpler method to substitute this function, cause A is stolen, B is for sure too computationally expensive
static int calc_baseline_and_numbits(unsigned int num_states_total, unsigned int num_states_symbol, unsigned int state_number, unsigned short int* a, unsigned char* b) {
    if (num_states_symbol == 0) {
        *a = 0, *b = 0;
		return ZSTD_NO_ERROR;
    }

	unsigned int num_state_slices = (1U << (highest_bit(num_states_symbol) - 1) == num_states_symbol) ? num_states_symbol : 1U << highest_bit(num_states_symbol); //always power of two
    unsigned int num_double_width_state_slices = num_state_slices - num_states_symbol; //leftovers to the power of two need to be distributed
    unsigned int num_single_width_state_slices = num_states_symbol - num_double_width_state_slices; //these will not receive a double width slice of states
    unsigned int slice_width = num_states_total / num_state_slices; //size of a single width slice of states
    
	if (slice_width == 0) {
		WARNING_LOG("Slice width cannot be 0.\n");
		return -ZSTD_CORRUPTED_DATA;
	}

	unsigned int num_bits = highest_bit(slice_width) - 1; //number of bits needed to read for one slice

    if (state_number < num_double_width_state_slices) {
        unsigned int baseline = num_single_width_state_slices * slice_width + state_number * slice_width * 2;
        *a = baseline, *b = num_bits + 1;
    } else {
        unsigned int index_shifted = state_number - num_double_width_state_slices;
        *a = (index_shifted * slice_width), *b = num_bits;
    }

	return ZSTD_NO_ERROR;
}

static int read_probabilities(BitStream* compressed_bit_stream, unsigned char table_log, unsigned char max_symbol, short int** frequencies, unsigned short int* probabilities_cnt) {
	if (table_log > FSE_TABLELOG_ABSOLUTE_MAX) return -ZSTD_TABLE_LOG_TOO_LARGE;
	int remaining = (1 << table_log) + 1;

	*frequencies = (short int*) qcow_realloc(*frequencies, (FSE_MAX_SYMBOL_VALUE + 1U) * sizeof(short int));
	if (*frequencies == NULL) {
		WARNING_LOG("Failed to allocate frequencies.\n");
		return -ZSTD_IO_ERROR;
	}
	mem_set(*frequencies, 0, (FSE_MAX_SYMBOL_VALUE + 1U) * sizeof(short int));
	*probabilities_cnt = 0;

	unsigned short int freq_cum_sum = 0;
	unsigned char zero_repeat = FALSE;
	while (TRUE) {
		if (zero_repeat) {
			unsigned char repeat = bitstream_read_bits(compressed_bit_stream, 2);
			*probabilities_cnt += repeat;
			if (repeat == 3) continue;
		}

		unsigned char nb_bits = highest_bit(remaining);
		unsigned short int max = (1 << (nb_bits - 1)) - 1;
		unsigned short int low_threshold = ((1 << nb_bits) - 1) - remaining;

		short int value = bitstream_read_bits(compressed_bit_stream, nb_bits);
		short int small_count = value & max;
		if (small_count < low_threshold) {
			bitstream_unread_bit(compressed_bit_stream);
			value = small_count;
		} else if (value > max) value -= low_threshold;

		value--; // Prediction = value - 1
		if (value < -1 || remaining <= 1) {
			QCOW_SAFE_FREE(frequencies);
			WARNING_LOG("Predictions cannot be less than 1: %d\n", value);
			return -ZSTD_CORRUPTED_DATA;
		}
		
		(*frequencies)[(*probabilities_cnt)++] = value; 
		freq_cum_sum += abs(value);
		remaining -= abs(value);

		zero_repeat = !value;

		if (*probabilities_cnt > max_symbol + 1 || freq_cum_sum >= (1 << table_log)) break;
	}

	if (*probabilities_cnt > max_symbol + 1) {
		QCOW_SAFE_FREE(*frequencies); 
		WARNING_LOG("Probabilities_cnt %u > %u max_symbol_value.\n", *probabilities_cnt, max_symbol + 1);
		return -ZSTD_MAX_SYMBOL_VALUE_TOO_SMALL;
	} else if (freq_cum_sum != (1 << table_log)) {
		QCOW_SAFE_FREE(*frequencies); 
		WARNING_LOG("Freq_cum_sum %u != %u expected frequencies count.\n", freq_cum_sum, (1 << table_log));
		return -ZSTD_CORRUPTED_DATA;
	}

	*frequencies = (short int*) qcow_realloc(*frequencies, *probabilities_cnt * sizeof(short int));
	if (*frequencies == NULL) {
		WARNING_LOG("Failed to allocate frequencies.\n");
		return -ZSTD_IO_ERROR;
	}

	if (compressed_bit_stream -> bit_pos) skip_to_next_byte(compressed_bit_stream); // Any remaining bit within the last byte is simply unused.

	return -ZSTD_NO_ERROR;
}

static int fse_build_table(unsigned char table_log, short int* frequencies, unsigned short int probabilities_cnt, FSETableEntry** fse_table) {
	if (table_log > FSE_TABLELOG_ABSOLUTE_MAX) return -ZSTD_TABLE_LOG_TOO_LARGE;

	// Build the decoding table from those probabilites
	unsigned short int table_size = 1 << table_log; 
	*fse_table = (FSETableEntry*) qcow_realloc(*fse_table, table_size * sizeof(FSETableEntry));
	if (*fse_table == NULL) {
		WARNING_LOG("Failed to allocate the fse table.\n");
		return -ZSTD_IO_ERROR;
	}

	// Start by assigning the -1 (also called "less than 1") probabilities' symbols, to the bottom of the table
	unsigned short int negative_index = table_size;
	for (unsigned short int i = 0; i < probabilities_cnt; ++i) {
		if (frequencies[i] == -1) {
			negative_index--;
			(*fse_table)[negative_index].symbol = i;
			(*fse_table)[negative_index].baseline = 0;
			(*fse_table)[negative_index].nb_bits = table_log;
		}	
	}

	unsigned short int update_pos = (table_size >> 1) + (table_size >> 3) + 3;
	unsigned short int tab_pos = 0;
	for (unsigned short int i = 0; i < probabilities_cnt; ++i) {
		for (short int j = 0; j < frequencies[i]; ++j) {
			(*fse_table)[tab_pos].symbol = i;
			tab_pos = (tab_pos + update_pos) & (table_size - 1);
			while (tab_pos >= negative_index) tab_pos = (tab_pos + update_pos) & (table_size - 1);
		}
	}
	
	if (tab_pos != 0) {
		QCOW_SAFE_FREE(*fse_table);
		WARNING_LOG("Tab pos didn't go back to 0: %u.\n", tab_pos);
		return -ZSTD_CORRUPTED_DATA;
	}
	
	unsigned int* symbol_counter = (unsigned int*) qcow_calloc(probabilities_cnt, sizeof(unsigned int));
	if (symbol_counter == NULL) {
		QCOW_SAFE_FREE(*fse_table);
		WARNING_LOG("Failed to allocate symbol counter buffer.\n");
		return -ZSTD_IO_ERROR;
	}

	for (unsigned short int i = 0; i < negative_index; ++i) {
		unsigned char symbol = (*fse_table)[i].symbol;
		short int prob = frequencies[symbol];

		unsigned int symbol_count = symbol_counter[symbol];
		calc_baseline_and_numbits(table_size, prob, symbol_count, &((*fse_table)[i].baseline), &((*fse_table)[i].nb_bits));

		if ((*fse_table)[i].nb_bits > table_log) {
			QCOW_MULTI_FREE(*fse_table, symbol_counter);
			WARNING_LOG("An error occurred while building the fse table.\n");
			return -ZSTD_CORRUPTED_DATA;
		}

		symbol_counter[symbol]++;
	}

	QCOW_SAFE_FREE(symbol_counter);
		
	return ZSTD_NO_ERROR;
}

static int read_weights(BitStream* compressed_bit_stream, unsigned char* table_log, unsigned char** weights, unsigned short int* weights_cnt) {
	int err = ZSTD_NO_ERROR;
	unsigned char header_byte = SAFE_BYTE_READ_WITH_CAST(compressed_bit_stream, sizeof(unsigned char), 1, unsigned char, header_byte, 0);
	
	// Use FSE-Decoding	
	if (header_byte < 128) { 
		unsigned char* compressed_literals_stream = SAFE_BYTE_READ(compressed_bit_stream, sizeof(unsigned char), header_byte, compressed_literals_stream);
		BitStream literals_compressed_bit_stream = CREATE_BIT_STREAM(compressed_literals_stream, header_byte);
		*table_log = bitstream_read_bits(&literals_compressed_bit_stream, 4) + 5;
		if (*table_log > 6) {
			WARNING_LOG("Table log exceeds the maximum value of 6: %u.\n", *table_log);
			return -ZSTD_CORRUPTED_DATA;
		}

		short int* frequencies = NULL;
		unsigned short int probabilities_cnt = 0;
		if ((err = read_probabilities(&literals_compressed_bit_stream, *table_log, FSE_MAX_SYMBOL_VALUE, &frequencies, &probabilities_cnt)) < 0) {
			WARNING_LOG("An error occurred while reading the probabilities distribution.\n");
			return err;
		}

		FSETableEntry* fse_table = NULL;
		if ((err = fse_build_table(*table_log, frequencies, probabilities_cnt, &fse_table))) {
			QCOW_SAFE_FREE(frequencies);
			WARNING_LOG("An error occurred while building the FSE Table.\n");
			return err;
		}
		
		QCOW_SAFE_FREE(frequencies);

		// Decode the fse encoded weights
		BitStream weights_compressed_bit_stream = CREATE_REVERSED_BIT_STREAM(literals_compressed_bit_stream.stream + literals_compressed_bit_stream.byte_pos, header_byte - literals_compressed_bit_stream.byte_pos, -(*table_log));
		SKIP_PADDING(err, &weights_compressed_bit_stream);	
		if (err < 0) {
			QCOW_SAFE_FREE(fse_table);
			return err;
		}

		unsigned short int even_state = reversed_bitstream_read_bits(&weights_compressed_bit_stream, *table_log);
		unsigned short int odd_state = reversed_bitstream_read_bits(&weights_compressed_bit_stream, *table_log);
		
		while (TRUE) {
			*weights = (unsigned char*) qcow_realloc(*weights, sizeof(unsigned char) * (*weights_cnt + 2));
			if (*weights == NULL) {
				QCOW_SAFE_FREE(fse_table);
				WARNING_LOG("Failed to qcow_reallocate the weights.\n");
				return -ZSTD_IO_ERROR;
			}

			(*weights)[*weights_cnt] = fse_table[even_state].symbol;
			UPDATE_FSE_STATE(even_state, fse_table, weights_compressed_bit_stream);
			(*weights_cnt)++;
			if ((*weights)[*weights_cnt - 1] > MAXIMUM_CODE_LENGTH) {
				QCOW_MULTI_FREE(fse_table, *weights);
				WARNING_LOG("An error occurred while decoding the encoded weights.\n");
				return -ZSTD_CORRUPTED_DATA;
			}

			if (weights_compressed_bit_stream.bit_pos < 0) {
				(*weights)[(*weights_cnt)++] = fse_table[odd_state].symbol;
				if ((*weights)[*weights_cnt - 1] > MAXIMUM_CODE_LENGTH) {
					QCOW_MULTI_FREE(fse_table, *weights);
					WARNING_LOG("An error occurred while decoding the encoded weights.\n");
					return -ZSTD_CORRUPTED_DATA;
				}
				break;
			}
			
			(*weights)[(*weights_cnt)++] = fse_table[odd_state].symbol;
			UPDATE_FSE_STATE(odd_state, fse_table, weights_compressed_bit_stream);
			if ((*weights)[*weights_cnt - 1] > MAXIMUM_CODE_LENGTH) {
				QCOW_MULTI_FREE(fse_table, *weights);
				WARNING_LOG("An error occurred while decoding the encoded weights.\n");
				return -ZSTD_CORRUPTED_DATA;
			}
			
			if (weights_compressed_bit_stream.bit_pos < 0) {
				*weights = (unsigned char*) qcow_realloc(*weights, sizeof(unsigned char) * (++(*weights_cnt)));
				if (*weights == NULL) {
					QCOW_SAFE_FREE(fse_table);
					WARNING_LOG("Failed to qcow_reallocate the weights.\n");
					return -ZSTD_IO_ERROR;
				}

				(*weights)[*weights_cnt - 1] = fse_table[even_state].symbol;
				if ((*weights)[*weights_cnt - 1] > MAXIMUM_CODE_LENGTH) {
					QCOW_MULTI_FREE(fse_table, *weights);
					WARNING_LOG("An error occurred while decoding the encoded weights.\n");
					return -ZSTD_CORRUPTED_DATA;
				}
				break;
			}
		}
			
		QCOW_SAFE_FREE(fse_table);
		
		if (*weights_cnt > FSE_MAX_SYMBOL_VALUE) {
			QCOW_SAFE_FREE(*weights);
			return -ZSTD_TOO_MANY_LITERALS;
		}
	} else {
		*weights_cnt = header_byte - 127;
		*weights = (unsigned char*) qcow_realloc(*weights, sizeof(unsigned char) * (*weights_cnt));
		if (*weights == NULL) {
			WARNING_LOG("Failed to qcow_reallocate the weights.\n");
			return -ZSTD_IO_ERROR;
		}

		unsigned char temp_byte = SAFE_BYTE_READ_WITH_CAST(compressed_bit_stream, sizeof(unsigned char), 1, unsigned char, temp_byte, 0, *weights);
		for (unsigned short int i = 0; i < *weights_cnt; ++i) {
			if ((i % 2) == 0) (*weights)[i] = (temp_byte >> 4) & 0x0F;
			else {
				(*weights)[i] = temp_byte & 0x0F;
				if ((i + 1) < *weights_cnt) {
					temp_byte = SAFE_BYTE_READ_WITH_CAST(compressed_bit_stream, sizeof(unsigned char), 1, unsigned char, temp_byte, 0, *weights);
				}
			}
		}
	}

	return ZSTD_NO_ERROR;
}

static int build_huff_table(BitStream* compressed_bit_stream, Workspace* workspace) {
	int err = 0;	
	unsigned short int weights_cnt = 0;
	unsigned char* weights = NULL;

	workspace -> hf_tree_desc_size = compressed_bit_stream -> byte_pos;
	if ((err = read_weights(compressed_bit_stream, &(workspace -> table_log), &weights, &weights_cnt)) < 0) {
		WARNING_LOG("An error occurred while reading the weights!\n");
		return err;
	}

	workspace -> hf_tree_desc_size = compressed_bit_stream -> byte_pos - workspace -> hf_tree_desc_size;

	unsigned int exp_weights_cnt = 0;	
	for (unsigned short int i = 0; i < weights_cnt; ++i) {
		if (weights[i] > 0) exp_weights_cnt += 1U << (weights[i] - 1);
	}
	
	// Infer the last weight and reconstruct the table for decoding of Huffman *state-based* Coding
	// NOTE for Adventurers: Don't be fooled by those monkeys at Meta that specify "Huffman Tree" in their RFC 8878 (sponsored as official reference of ZSTD), and instead use a state-based approach
	// Furthermore, thanks to "zstd-rs" (at "https://github.com/KillingSpark/zstd-rs") for showing what they were actually doing inside their jungle mess of code.
	workspace -> max_nb_bits = highest_bit(exp_weights_cnt);
	weights = qcow_realloc(weights, sizeof(unsigned char) * (++weights_cnt));
	if (weights == NULL) {
		WARNING_LOG("Failed to qcow_reallocate the weights.\n");
		return -ZSTD_IO_ERROR;
	}
	
	weights[weights_cnt - 1] = highest_bit((1 << workspace -> max_nb_bits) - exp_weights_cnt); // Add the hidden literal and its weight
	
	unsigned short int hf_literals_size = 1 << workspace -> max_nb_bits;
	unsigned short int hf_literals_cnt = 0;
	workspace -> hf_literals = (ZSTDHfEntry*) qcow_realloc(workspace -> hf_literals, hf_literals_size * sizeof(ZSTDHfEntry));
	if (workspace -> hf_literals == NULL) {
		QCOW_SAFE_FREE(weights);
		WARNING_LOG("Failed to qcow_reallocate the huff table for literals.\n");
		return -ZSTD_IO_ERROR;
	}
	
	mem_set(workspace -> hf_literals, 0, hf_literals_size * sizeof(ZSTDHfEntry));

	for (unsigned short int i = 0; i < weights_cnt; ++i) {
		if (weights[i] == 0) continue;
		unsigned short int j = 0;
		unsigned char nb_bits = workspace -> max_nb_bits + 1 - weights[i];
		while (j < hf_literals_size && ((workspace -> hf_literals)[j].nb_bits >= nb_bits && (workspace -> hf_literals)[j].symbol < i)) ++j;
		unsigned short int symbols_cnt = 1 << (weights[i] - 1);
		if (j < hf_literals_cnt) mem_move(workspace -> hf_literals + j + symbols_cnt, workspace -> hf_literals + j, (hf_literals_cnt - j) * sizeof(ZSTDHfEntry));
		for (unsigned short int s = j; s < j + symbols_cnt; ++s) (workspace -> hf_literals)[s].symbol = i, (workspace -> hf_literals)[s].nb_bits = nb_bits; 
		hf_literals_cnt += symbols_cnt;
	}
	
	QCOW_SAFE_FREE(weights);
	
	if (hf_literals_size != hf_literals_cnt) {
		QCOW_SAFE_FREE(workspace -> hf_literals);
		WARNING_LOG("Mismatch in the hf literals size: %u != %u (expected size).\n", hf_literals_size, hf_literals_cnt);
		return -ZSTD_CORRUPTED_DATA;
	}
	
	return ZSTD_NO_ERROR;
}

static int huff_decode_stream(BitStream* literals_stream, unsigned short int hf_literals_size, unsigned int regenerated_size, Workspace* workspace) {
	int err = 0;
	SKIP_PADDING(err, literals_stream);
	if (err < 0) return err;

	unsigned short int hf_state = reversed_bitstream_read_bits(literals_stream, workspace -> max_nb_bits);
	while ((literals_stream -> bit_pos > -(workspace -> max_nb_bits)) && (workspace -> literals_cnt < regenerated_size)) {
		(workspace -> literals)[(workspace -> literals_cnt)++] = (workspace -> hf_literals)[hf_state].symbol;
		UPDATE_HF_STATE(hf_state, workspace -> hf_literals, hf_literals_size, literals_stream);
	}

	if (literals_stream -> byte_pos > 0) {
		WARNING_LOG("Expected empty bitstream, but got still %u bytes inside.\n", literals_stream -> byte_pos);
		return -ZSTD_CORRUPTED_DATA;
	}
	
	return ZSTD_NO_ERROR;
}

static int decode_literals(BitStream* compressed_bit_stream, Workspace* workspace, LiteralsSectionHeader lsh) {
	// Decode the literals from the stream/streams
	int err = 0;
	if (lsh.literals_block_type == COMPRESSED_LITERALS_BLOCK) {
		if ((err = build_huff_table(compressed_bit_stream, workspace)) < 0) {
			WARNING_LOG("Failed to build the huffman table.\n");
			return -ZSTD_CORRUPTED_DATA;
		}
	} else {
		workspace -> hf_tree_desc_size = 0;
		if (workspace -> hf_literals == NULL) {
			WARNING_LOG("Uninitialized hf_literals for treeless compressed block, either a IO error, or the stream is corrupted.\n");
			return -ZSTD_CORRUPTED_DATA;
		}
	}
	
	unsigned short int hf_literals_size = 1 << workspace -> max_nb_bits;
	unsigned int total_streams_size = lsh.compressed_size - workspace -> hf_tree_desc_size;
	if (lsh.streams_cnt == 1) {
		unsigned char* literals_stream = SAFE_BYTE_READ(compressed_bit_stream, sizeof(unsigned char), total_streams_size, literals_stream, workspace -> hf_literals);
		BitStream literals_bit_stream = CREATE_REVERSED_BIT_STREAM(literals_stream, total_streams_size, -(workspace -> max_nb_bits));
		if (((err = huff_decode_stream(&literals_bit_stream, hf_literals_size, lsh.regenerated_size, workspace)) < 0) || (workspace -> literals_cnt != lsh.regenerated_size)) {
			QCOW_SAFE_FREE(workspace -> hf_literals);
			WARNING_LOG("An error occurred while decoding the literals huff encoded stream.\n");
			return err;
		}
	} else {
		unsigned short int streams_size[4] = {0};
		unsigned char* streams_size_data = SAFE_BYTE_READ(compressed_bit_stream, sizeof(unsigned short int), 3, streams_size_data, workspace -> hf_literals);
		mem_cpy(streams_size, streams_size_data, sizeof(unsigned short int) * 3);
		streams_size[3] = INFER_LAST_STREAM_SIZE(total_streams_size, streams_size);

		for (unsigned char i = 0; i < 4; ++i) {
			unsigned char* literals_sub_stream = SAFE_BYTE_READ(compressed_bit_stream, sizeof(unsigned char), streams_size[i], literals_sub_stream, workspace -> hf_literals);
			BitStream literals_sub_bit_stream = CREATE_REVERSED_BIT_STREAM(literals_sub_stream, streams_size[i], -(workspace -> max_nb_bits));
			if (compressed_bit_stream -> error) {
				QCOW_SAFE_FREE(workspace -> hf_literals);
				WARNING_LOG("Not enough data for the streams.\n");
				return -ZSTD_CORRUPTED_DATA;
			} else if (((err = huff_decode_stream(&literals_sub_bit_stream, hf_literals_size, lsh.regenerated_size, workspace)) < 0) || (literals_sub_bit_stream.bit_pos != -(workspace -> max_nb_bits))) {
				QCOW_SAFE_FREE(workspace -> hf_literals);
				WARNING_LOG("An error occurred while decoding the literals huff encoded in the substream '%u'.\n", i + 1);
				return -ZSTD_CORRUPTED_DATA;
			}	
		}
		
		if (workspace -> literals_cnt != lsh.regenerated_size) {
			QCOW_SAFE_FREE(workspace -> hf_literals);
			WARNING_LOG("Size mismatch between literals_cnt and the expected regenerated size: %u != %u .\n", workspace -> literals_cnt, lsh.regenerated_size);
			return -ZSTD_CORRUPTED_DATA;
		}
	}

	return err;
}

static int parse_literals_section(BitStream* compressed_bit_stream, Workspace* workspace) {
	LiteralsSectionHeader lsh = {0};
	lsh.literals_block_type = bitstream_read_bits(compressed_bit_stream, 2);
	DEBUG_LOG("literals_block_type: '%s'\n", literals_blocks_type_str[lsh.literals_block_type]);
	unsigned char size_format = bitstream_read_bits(compressed_bit_stream, 2);
	
	int err = 0;
	if (lsh.literals_block_type == RAW_LITERALS_BLOCK || lsh.literals_block_type == RLE_LITERALS_BLOCK) {
		if (size_format == 0 || size_format == 2) lsh.regenerated_size = (bitstream_read_bits(compressed_bit_stream, 4) << 1) + (size_format >> 1); 
		else if (size_format == 1) lsh.regenerated_size = bitstream_read_bits(compressed_bit_stream, 12); 
		else lsh.regenerated_size = bitstream_read_bits(compressed_bit_stream, 20); 
		DEBUG_LOG("regenerated_size: %u\n", lsh.regenerated_size);
		
		workspace -> literals_cnt = 0;
		QCOW_SAFE_FREE(workspace -> literals);
		workspace -> literals = (unsigned char*) qcow_calloc(lsh.regenerated_size, sizeof(unsigned char));
		if (workspace -> literals == NULL) {
			WARNING_LOG("Failed to allocate literals buffer.\n");
			return -ZSTD_IO_ERROR;
		}

		if (lsh.literals_block_type == RAW_LITERALS_BLOCK) {
			unsigned char* raw_literals_block_data = SAFE_BYTE_READ(compressed_bit_stream, sizeof(unsigned char), lsh.regenerated_size, raw_literals_block_data);
			mem_cpy(workspace -> literals, raw_literals_block_data, lsh.regenerated_size);
		} else { 
			unsigned char rle_literal_val = SAFE_BYTE_READ_WITH_CAST(compressed_bit_stream, sizeof(unsigned char), 1, unsigned char, rle_literal_val, 0);
			mem_set(workspace -> literals, rle_literal_val, lsh.regenerated_size);
		}
	} else {
		lsh.regenerated_size = bitstream_read_bits(compressed_bit_stream, GET_LITERALS_FIELDS_BIT_SIZE(size_format));
		lsh.compressed_size = bitstream_read_bits(compressed_bit_stream, GET_LITERALS_FIELDS_BIT_SIZE(size_format));
		lsh.streams_cnt = size_format == 0 ? 1 : 4;
		
		QCOW_SAFE_FREE(workspace -> literals);
		workspace -> literals_cnt = 0;
		workspace -> literals = (unsigned char*) qcow_calloc(lsh.regenerated_size, sizeof(unsigned char));
		if (workspace -> literals == NULL) {
			WARNING_LOG("Failed to allocate literals buffer.\n");
			return -ZSTD_IO_ERROR;
		} else if ((err = decode_literals(compressed_bit_stream, workspace, lsh)) < 0) {
			WARNING_LOG("An error occurred while decoding the literals.\n");
			return err;
		}
	}

	workspace -> literals_cnt = lsh.regenerated_size;

	return ZSTD_NO_ERROR;
}

// ---------------------------------------
//  Sequence Parsing and Decoding Section
// ---------------------------------------
#define init_length_type(err, compressed_bit_stream, type_len_mode, type_table_log, pred_frequencies, pred_table_log, type_max_log, type_max_symbol, type_fse_table, type_rle)	\
	do { 																																        								\
		switch (type_len_mode) { 																									            								\
			case PREDEFINED_MODE: { 																										  									\
				if (type_fse_table != NULL) QCOW_SAFE_FREE(type_fse_table); 																											\
				if ((err = fse_build_table(pred_table_log, (short int*)pred_frequencies, QCOW_ARR_SIZE(pred_frequencies), &(type_fse_table))) < 0) {									\
					WARNING_LOG("An error occurred while building the table for predefined.\n"); 																				\
					return err; 																											    								\
				} 																																								\
				type_table_log = pred_table_log; 																																\
				type_rle = NOT_USING_RLE;	 																																	\
				break; 																																							\
			}  																																									\
			case FSE_COMPRESSED_MODE: { 																																		\
				if (type_fse_table != NULL) QCOW_SAFE_FREE(type_fse_table); 																											\
				type_table_log = bitstream_read_bits(compressed_bit_stream, 4) + 5;				 																				\
				if (type_table_log > type_max_log) { 																															\
					WARNING_LOG("Table log exceeds the maximum value of %u: %u.\n", type_max_log, type_table_log); 																\
					return -ZSTD_CORRUPTED_DATA; 																																\
				}		 																																						\
				short int* frequencies = NULL; 																																	\
				unsigned short int probabilities_cnt = 0; 																														\
				if ((err = read_probabilities(compressed_bit_stream, type_table_log, type_max_symbol, &frequencies, &probabilities_cnt)) < 0) { 								\
					WARNING_LOG("An error occurred while reading the probabilities for predefined.\n"); 																		\
					return err; 																																				\
				} 																																								\
				if ((err = fse_build_table(type_table_log, frequencies, probabilities_cnt, &(type_fse_table))) < 0) { 															\
					QCOW_SAFE_FREE(frequencies);																																		\
					WARNING_LOG("An error occurred while building the table for predefined.\n"); 																				\
					return err; 																																				\
				} 																																								\
				QCOW_SAFE_FREE(frequencies);  																																		\
				type_rle = NOT_USING_RLE;	 																																	\
				break; 																																							\
			} 																																									\
			case RLE_MODE: { 																																					\
				type_rle = SAFE_BYTE_READ_WITH_CAST(compressed_bit_stream, sizeof(unsigned char), 1, unsigned char, type_rle, 0); 												\
				break; 																																							\
			} 																																									\
			case REPEAT_MODE: { 																																				\
				if (type_fse_table == NULL) {																																	\
					WARNING_LOG("Uninitialized " #type_fse_table " either due to a previous IO error, or the stream is actually corrupted.\n"); 								\
					return -ZSTD_CORRUPTED_DATA;																																\
				}																																								\
				break; 																																							\
			} 																																									\
		} 																																										\
	} while(FALSE)

static int decode_sequences(BitStream* sequence_compressed_bit_stream, Workspace* workspace) {
	unsigned int ll_state = 0;
	unsigned int ol_state = 0;
	unsigned int ml_state = 0;
	
	if (workspace -> sequence_section.ll_rle == NOT_USING_RLE) ll_state = reversed_bitstream_read_bits(sequence_compressed_bit_stream, workspace -> sequence_section.ll_table_log);	
	if (workspace -> sequence_section.ol_rle == NOT_USING_RLE) ol_state = reversed_bitstream_read_bits(sequence_compressed_bit_stream, workspace -> sequence_section.ol_table_log);	
	if (workspace -> sequence_section.ml_rle == NOT_USING_RLE) ml_state = reversed_bitstream_read_bits(sequence_compressed_bit_stream, workspace -> sequence_section.ml_table_log);	
	
	for (unsigned int i = 0; i < workspace -> sequence_len; ++i) {
		unsigned int ll_code = workspace -> sequence_section.ll_rle == NOT_USING_RLE ? (workspace -> sequence_section.ll_fse_table)[ll_state].symbol : workspace -> sequence_section.ll_rle;
		unsigned int ml_code = workspace -> sequence_section.ml_rle == NOT_USING_RLE ? (workspace -> sequence_section.ml_fse_table)[ml_state].symbol : workspace -> sequence_section.ml_rle;
		unsigned int ol_code = workspace -> sequence_section.ol_rle == NOT_USING_RLE ? (workspace -> sequence_section.ol_fse_table)[ol_state].symbol : workspace -> sequence_section.ol_rle;
		
		LengthCode ll_actual_codes = *QCOW_CAST_PTR(ll_codes[ll_code], LengthCode);
		LengthCode ml_actual_codes = *QCOW_CAST_PTR(ml_codes[ml_code], LengthCode);
	
		if (ol_code > MAX_OL_CODE) {
			WARNING_LOG("Offset Length code cannot be bigger than %u: %u\n", MAX_OL_CODE, ol_code);
			return -ZSTD_CORRUPTED_DATA;
		}
	
        unsigned int offset = reversed_bitstream_read_bits(sequence_compressed_bit_stream, ol_code) + (1U << ol_code);
		unsigned int ml_add = reversed_bitstream_read_bits(sequence_compressed_bit_stream, ml_actual_codes.num_bits);
		unsigned int ll_add = reversed_bitstream_read_bits(sequence_compressed_bit_stream, ll_actual_codes.num_bits);
			
		if (offset == 0) {
			WARNING_LOG("Offset cannot be 0.\n");
			return -ZSTD_CORRUPTED_DATA;
		}
		
		(workspace -> sequences)[i] = (Sequence) {
			.ll_value = ll_actual_codes.value + ll_add,
			.ml_value = ml_actual_codes.value + ml_add,
			.ol_value = offset
		};
		
		if ((i + 1) < workspace -> sequence_len) {
			if (workspace -> sequence_section.ll_rle == NOT_USING_RLE) UPDATE_FSE_STATE(ll_state, workspace -> sequence_section.ll_fse_table, *sequence_compressed_bit_stream);
			if (workspace -> sequence_section.ml_rle == NOT_USING_RLE) UPDATE_FSE_STATE(ml_state, workspace -> sequence_section.ml_fse_table, *sequence_compressed_bit_stream);
			if (workspace -> sequence_section.ol_rle == NOT_USING_RLE) UPDATE_FSE_STATE(ol_state, workspace -> sequence_section.ol_fse_table, *sequence_compressed_bit_stream);
		}

		if (sequence_compressed_bit_stream -> error) {
			QCOW_SAFE_FREE(workspace -> sequences);
			WARNING_LOG("Tried to read after the end of the stream.\n");
			return -ZSTD_CORRUPTED_DATA;
		}
	}	
	
	if (!IS_REVERSED_EOS(sequence_compressed_bit_stream)) {
		QCOW_SAFE_FREE(workspace -> sequences);
		WARNING_LOG("Stream not empty.\n");
		PRINT_BIT_STREAM_INFO(sequence_compressed_bit_stream);
		return -ZSTD_CORRUPTED_DATA;
	}
	
	return ZSTD_NO_ERROR;
}

static int parse_sequence_section(BitStream* compressed_bit_stream, Workspace* workspace) {
	workspace -> sequence_len = SAFE_BYTE_READ_WITH_CAST(compressed_bit_stream, sizeof(unsigned char), 1, unsigned char, workspace -> sequence_len, 0);
	
	if (workspace -> sequence_len > 127) {
		unsigned char first_byte = workspace -> sequence_len;
		if (first_byte < 255) {
			workspace -> sequence_len = SAFE_BYTE_READ_WITH_CAST(compressed_bit_stream, sizeof(unsigned char), 1, unsigned char, workspace -> sequence_len, 0);
		} else {
			workspace -> sequence_len = SAFE_BYTE_READ_WITH_CAST(compressed_bit_stream, sizeof(unsigned char), 2, unsigned short int, workspace -> sequence_len, 0);
		}
		workspace -> sequence_len += first_byte < 255 ? ((first_byte - 128) << 8) : 0x7F00;
	}
	
	DEBUG_LOG("sequence_len: %u\n", workspace -> sequence_len);
	
	if (workspace -> sequence_len == 0) {
		if (!IS_EOS(compressed_bit_stream)) {
			WARNING_LOG("Expected end of stream, but the bitstream is not empty: %u bytes left.\n", compressed_bit_stream -> size - compressed_bit_stream -> byte_pos);
			return -ZSTD_CORRUPTED_DATA;
		}
		return ZSTD_NO_ERROR;
	}

	SymbolCompressionModes symbol_compression_modes = SAFE_BYTE_READ_WITH_CAST(compressed_bit_stream, sizeof(SymbolCompressionModes), 1, SymbolCompressionModes, symbol_compression_modes, {0});
	if (symbol_compression_modes.reserved != 0) {
		WARNING_LOG("Use of symbol compression mode reserved field.\n");
		return -ZSTD_RESERVED_FIELD;
	}
	
	DEBUG_LOG("SymbolCompressionModes: (0x%X)\n", *QCOW_CAST_PTR(&symbol_compression_modes, unsigned char));
	DEBUG_LOG(" - literals_length_mode: '%s'\n", compression_modes_str[symbol_compression_modes.literals_len_mode]);
	DEBUG_LOG(" - offset_mode: '%s'\n", compression_modes_str[symbol_compression_modes.offset_mode]);
	DEBUG_LOG(" - match_length_mode: '%s'\n", compression_modes_str[symbol_compression_modes.match_len_mode]);
	
	// Build the FSE table for each type from predefined distribution or build it from decoded distribution, for RLE recover the single value that compose the entire table
	// For Repeat nothing has to be done, at the moment we are not able to take track of previously used tables
	// Furthermore, for Repeat just check that it can be used, like if this is not the first block and similars
	int err = 0;
	SequenceSection* sequence_section = &(workspace -> sequence_section);
	init_length_type(err, compressed_bit_stream, symbol_compression_modes.literals_len_mode, sequence_section -> ll_table_log, ll_pred_frequencies, PRED_LL_TABLE_LOG, LL_MAX_LOG, MAX_LL_CODE, sequence_section -> ll_fse_table, sequence_section -> ll_rle);
	init_length_type(err, compressed_bit_stream, symbol_compression_modes.offset_mode,       sequence_section -> ol_table_log, ol_pred_frequencies, PRED_OL_TABLE_LOG, OL_MAX_LOG, MAX_OL_CODE, sequence_section -> ol_fse_table, sequence_section -> ol_rle);
	init_length_type(err, compressed_bit_stream, symbol_compression_modes.match_len_mode,    sequence_section -> ml_table_log, ml_pred_frequencies, PRED_ML_TABLE_LOG, ML_MAX_LOG, MAX_ML_CODE, sequence_section -> ml_fse_table, sequence_section -> ml_rle);

	unsigned int sequence_stream_size = compressed_bit_stream -> size - compressed_bit_stream -> byte_pos;
	unsigned char* sequence_compressed_stream = SAFE_BYTE_READ(compressed_bit_stream, sizeof(unsigned char), sequence_stream_size, sequence_compressed_stream);
	BitStream sequence_compressed_bit_stream = CREATE_REVERSED_BIT_STREAM(sequence_compressed_stream, sequence_stream_size, 0);
	
	SKIP_PADDING(err, &sequence_compressed_bit_stream);
	if (err < 0) {
		WARNING_LOG("An error occurred while parsing the sequence section.\n");
		return err;
	}
	
	// Parse the FSE Table as defined in decode_sequences_with_rle in zstd-rs, the price of few branches is feasible as at the moment we value more readability.
	// At each pass it will decode a Sequence, so we need a struct for the Sequences
	QCOW_SAFE_FREE(workspace -> sequences);
	workspace -> sequences = (Sequence*) qcow_calloc(workspace -> sequence_len, sizeof(Sequence));
	if (workspace -> sequences == NULL) {
		WARNING_LOG("Failed to allocate sequences buffer.\n");
		return -ZSTD_IO_ERROR;
	} else if ((err = decode_sequences(&sequence_compressed_bit_stream, workspace)) < 0) {
		deallocate_sequence_section(sequence_section);
		WARNING_LOG("An error occurred while decoding the sequences.\n");
		return err;
	}

	return ZSTD_NO_ERROR;
}

// ----------------------------
//  Sequence Execution Section
// ----------------------------
static unsigned int update_off_history(unsigned int* offset_history, unsigned int offset, unsigned int ll_value) {
	unsigned int actual_offset = 0;
	if (offset > 3) actual_offset = offset - 3;
	else {
		if (ll_value > 0) actual_offset = offset_history[offset - 1];
		else actual_offset = offset == 3 ? offset_history[0] - 1 : offset_history[offset];
	}

	if (offset >= 3 || (offset == 2 && ll_value == 0)) {
		offset_history[2] = offset_history[1];
    	offset_history[1] = offset_history[0];
		offset_history[0] = actual_offset;
	} else if ((offset == 1 && ll_value == 0) || (offset == 2 && ll_value > 0)) {
		offset_history[1] = offset_history[0];
        offset_history[0] = actual_offset;
	}
	
	return actual_offset;
}

static int sequence_execution(Workspace* workspace) {
	if (workspace -> sequence_len == 0) {
		mem_cpy(workspace -> frame_buffer + workspace -> frame_buffer_len, workspace -> literals, workspace -> literals_cnt);
		workspace -> frame_buffer_len += workspace -> literals_cnt;
		return ZSTD_NO_ERROR;
	}

	unsigned int literals_ind = 0;
	unsigned int sequence_cnt = 0;
	unsigned int original_frame_len = workspace -> frame_buffer_len;
	unsigned int* offset_history = workspace -> offset_history;
	if (offset_history == NULL) {
		WARNING_LOG("Uninitialized offset history.\n");
		return -ZSTD_IO_ERROR;
	}

	for (unsigned int i = 0; i < workspace -> sequence_len; ++i) {
		Sequence sequence = workspace -> sequences[i];

		if (sequence.ll_value > 0) {
			if (sequence.ll_value + literals_ind > workspace -> literals_cnt) {
				WARNING_LOG("Literals length value makes index out of range (%u > %u).\n", (sequence.ll_value + literals_ind), workspace -> literals_cnt);
				return -ZSTD_CORRUPTED_DATA;
			}
			mem_cpy(workspace -> frame_buffer + workspace -> frame_buffer_len, workspace -> literals + literals_ind, sequence.ll_value);
			literals_ind += sequence.ll_value;
			workspace -> frame_buffer_len += sequence.ll_value;
			sequence_cnt += sequence.ll_value;
		}

		unsigned int offset = update_off_history(offset_history, sequence.ol_value, sequence.ll_value);
		if (offset == 0) {
			WARNING_LOG("Actual offset cannot be zero.\n");
			return -ZSTD_CORRUPTED_DATA;
		}


		if (sequence.ml_value > 0) {
			unsigned int current_pos = workspace -> frame_buffer_len;
			if (((long int) current_pos - offset) < 0LL) {
				WARNING_LOG("Offset makes negative index into literals: %ld.\n", ((long int) current_pos - offset));
				return -ZSTD_CORRUPTED_DATA;
			}
			for (unsigned int i = 0; i < sequence.ml_value; ++i) (workspace -> frame_buffer)[(workspace -> frame_buffer_len)++] = (workspace -> frame_buffer)[current_pos + i - offset];
			sequence_cnt += sequence.ml_value;
		}
	}
	
	if (literals_ind < workspace -> literals_cnt) {
		mem_cpy(workspace -> frame_buffer + workspace -> frame_buffer_len, workspace -> literals + literals_ind, workspace -> literals_cnt - literals_ind);
		workspace -> frame_buffer_len += workspace -> literals_cnt - literals_ind;
		sequence_cnt += workspace -> literals_cnt - literals_ind;
	}

	if (sequence_cnt != (workspace -> frame_buffer_len - original_frame_len)) {
		WARNING_LOG("Size mismatch between sequence_cnt: %u and the decoded len: %u\n", sequence_cnt, (workspace -> frame_buffer_len - original_frame_len));
		return -ZSTD_CORRUPTED_DATA;
	}

	return ZSTD_NO_ERROR;
}

// ----------------------------------------------
//  Block and Frame Parsing and Decoding Section
// ----------------------------------------------
static int decompress_block(BitStream* compressed_bit_stream, Workspace* workspace) {
	int err = 0;
	if ((err = parse_literals_section(compressed_bit_stream, workspace)) < 0) {
		WARNING_LOG("An error occurred while parsing the literals section.\n");
		return err;
	}

	// Use the Literals to decode the sequence section
	if ((err = parse_sequence_section(compressed_bit_stream, workspace))) {
		WARNING_LOG("An error occurred while parsing the sequence section.\n");
		return err;
	}

	if ((err = sequence_execution(workspace)) < 0) {
		WARNING_LOG("An error occurred while performing sequence execution.\n");
		return err;
	}

	return ZSTD_NO_ERROR;
}

static int parse_block(BitStream* bit_stream, Workspace* workspace, unsigned int block_maximum_size) {
	BlockHeader block_header = SAFE_BYTE_READ_WITH_CAST(bit_stream, sizeof(BlockHeader), 1, BlockHeader, block_header, {0});
	DEBUG_LOG("BlockHeader: (0x%X)\n", *QCOW_CAST_PTR(&block_header, unsigned int));
	DEBUG_LOG(" - last_block: %u\n", block_header.last_block);
	DEBUG_LOG(" - block_type: '%s'\n", block_types_str[block_header.block_type]);
	DEBUG_LOG(" - block_size: %u\n", block_header.block_size);
	
	if (block_header.block_type == RESERVED_TYPE) return -ZSTD_RESERVED;
	
	if (block_header.block_type == RAW_BLOCK) {
		if (workspace -> frame_buffer_len + block_header.block_size) {
			workspace -> frame_buffer = (unsigned char*) qcow_realloc(workspace -> frame_buffer, (workspace -> frame_buffer_len + block_header.block_size) * sizeof(unsigned char));
			if (workspace -> frame_buffer == NULL) {
				WARNING_LOG("Failed to qcow_reallocate frame buffer.\n");
				return -ZSTD_IO_ERROR;
			}

			unsigned char* raw_block_data = SAFE_BYTE_READ(bit_stream, sizeof(unsigned char), block_header.block_size, raw_block_data);
			mem_cpy(workspace -> frame_buffer + workspace -> frame_buffer_len, raw_block_data, block_header.block_size * sizeof(unsigned char));
			workspace -> frame_buffer_len += block_header.block_size;
		}
	} else if (block_header.block_type == RLE_BLOCK) {
		workspace -> frame_buffer = (unsigned char*) qcow_realloc(workspace -> frame_buffer, (workspace -> frame_buffer_len + block_header.block_size) * sizeof(unsigned char));
		if (workspace -> frame_buffer == NULL) {
			WARNING_LOG("Failed to qcow_reallocate frame buffer.\n");
			return -ZSTD_IO_ERROR;
		}
		
		unsigned char rle_val = SAFE_BYTE_READ_WITH_CAST(bit_stream, sizeof(unsigned char), 1, unsigned char, rle_val, 0);
		mem_set(workspace -> frame_buffer + workspace -> frame_buffer_len, rle_val, block_header.block_size * sizeof(unsigned char));
		workspace -> frame_buffer_len += block_header.block_size;
	} else {
		workspace -> frame_buffer = (unsigned char*) qcow_realloc(workspace -> frame_buffer, (workspace -> frame_buffer_len + block_maximum_size) * sizeof(unsigned char));
		if (workspace -> frame_buffer == NULL) {
			WARNING_LOG("Failed to qcow_reallocate frame buffer.\n");
			return -ZSTD_IO_ERROR;
		}
		
		unsigned char* compressed_stream = SAFE_BYTE_READ(bit_stream, sizeof(unsigned char), block_header.block_size, compressed_stream);
		BitStream compressed_bit_stream = CREATE_BIT_STREAM(compressed_stream, block_header.block_size);
		int err = 0;
		if ((err = decompress_block(&compressed_bit_stream, workspace)) < 0) {
			QCOW_SAFE_FREE(workspace -> frame_buffer);
			WARNING_LOG("An error occurred while decompressing the block.\n");
			return err;
		}

		workspace -> frame_buffer = (unsigned char*) qcow_realloc(workspace -> frame_buffer, workspace -> frame_buffer_len * sizeof(unsigned char));
		if (workspace -> frame_buffer == NULL && workspace -> frame_buffer_len != 0) {
			WARNING_LOG("Failed to qcow_reallocate frame buffer.\n");
			return -ZSTD_IO_ERROR;
		}
	}

	return block_header.last_block; // Return the information to the frame parser
}	

static int parse_frames(BitStream* bit_stream, unsigned char** decompressed_data, unsigned int* decompressed_data_length) {
	unsigned int magic = SAFE_BYTE_READ_WITH_CAST(bit_stream, sizeof(unsigned int), 1, unsigned int, magic, 0);
	DEBUG_LOG("magic: 0x%X\n", magic);
	
	if (ZSTD_SKIPPABLE_FRAME_MAGIC_MIN <= magic && magic <= ZSTD_SKIPPABLE_FRAME_MAGIC_MAX) {
		unsigned int frame_len = SAFE_BYTE_READ_WITH_CAST(bit_stream, sizeof(unsigned int), 1, unsigned int, frame_len, 0);
		DEBUG_LOG("Skipping 'skippable frame' with length %u found!\n", frame_len);
		unsigned char* skipped_data = SAFE_BYTE_READ(bit_stream, sizeof(unsigned char), frame_len, skipped_data);
		UNUSED_VAR(skipped_data);
		return ZSTD_NO_ERROR;
	}

	if (magic != ZSTD_FRAME_MAGIC) {
		WARNING_LOG("Invalid magic: 0x%X != 0x%X (the expected magic)\n", magic, ZSTD_FRAME_MAGIC);
		return -ZSTD_INVALID_MAGIC;
	}
	
	// Frame Header
	FrameHeaderDescriptor fhd = SAFE_BYTE_READ_WITH_CAST(bit_stream, sizeof(FrameHeaderDescriptor), 1, FrameHeaderDescriptor, fhd, {0});
	print_fhd(fhd);

	if (fhd.reserved != 0) {
		WARNING_LOG("Used frame header descriptor reserved field.\n");
		return -ZSTD_RESERVED_FIELD;
	}

	unsigned long long int window_size = 0;
	if (!fhd.single_segment_flag) {
		WindowDescriptor window_descriptor = SAFE_BYTE_READ_WITH_CAST(bit_stream, sizeof(WindowDescriptor), 1, WindowDescriptor, window_descriptor, {0});
		unsigned char window_log = 10 + window_descriptor.exponent;
		unsigned long long int window_base = 1 << window_log;
		unsigned long long int window_add = (window_base / 8) * window_descriptor.mantissa;
		window_size = window_base + window_add;
	}
	
	if (fhd.dictionary_id_flag != 0) {
		WARNING_LOG("Sorry, but this decoder implementation does not support the use of dictionaries.\n");
		return -ZSTD_UNSUPPORTED_FEATURE;
	} 

	unsigned char frame_content_size_len = GET_FRAME_CONTENT_SIZE(fhd.frame_content_size_flag, fhd.single_segment_flag);
	DEBUG_LOG("frame_content_size_len: %u\n", frame_content_size_len);

	unsigned long long int frame_content_size = 0;
	if (frame_content_size_len) {
		unsigned char* frame_content_size_data = SAFE_BYTE_READ(bit_stream, frame_content_size_len, 1, frame_content_size_data);
		mem_cpy(&frame_content_size, frame_content_size_data, frame_content_size_len);
		if (frame_content_size_len == 2) frame_content_size += 256; 
	}
	
	if (fhd.single_segment_flag) window_size = frame_content_size;

	DEBUG_LOG("Frame Header:\n");
	DEBUG_LOG(" - window_size: %llu\n", window_size);
	DEBUG_LOG(" - frame_content_size: %llu\n", frame_content_size);
	
	// Define the workspace for this frame
	unsigned int offset_history[] = {1, 4, 8};
	Workspace workspace = {0};
	workspace.offset_history = offset_history;

	int err = 0;
	unsigned int blocks_cnt = 0;
	do {
		DEBUG_LOG("Parsing data block num %u\n", blocks_cnt);
		// Previous decoded data, up to a distance of Window_Size, or the beginning of the Frame, whichever is smaller. Single_Segment_Flag will be set in the latter case.
		if ((err = parse_block(bit_stream, &workspace, MAX(window_size, MAX_BLOCK_SIZE))) < 0) {
			deallocate_workspace(&workspace);
			WARNING_LOG("An error occurred while decoding a block.\n");
			return err;
		}
		blocks_cnt++;
	} while (err != 1);
	
	if (fhd.content_checksum_flag) {
		unsigned int frame_checksum = SAFE_BYTE_READ_WITH_CAST(bit_stream, sizeof(unsigned int), 1, unsigned int, frame_checksum, 0);
		DEBUG_LOG("frame checksum: 0x%X\n", frame_checksum);
		uint64_t decoded_checksum = xxhash64(workspace.frame_buffer, workspace.frame_buffer_len, 0) & 0xFFFFFFFF;
		if (decoded_checksum != frame_checksum) {
			DEBUG_LOG("data(%u): '%.*s'\n", workspace.frame_buffer_len, workspace.frame_buffer_len, workspace.frame_buffer);
			deallocate_workspace(&workspace);
			WARNING_LOG("The checksum of the frame doesn't match with the one found at the end of the frame (0x%lX != 0x%X).\n", decoded_checksum, frame_checksum);
			return -ZSTD_CHECKSUM_FAIL;
		}
	}
	
	if (workspace.frame_buffer_len) {
		*decompressed_data = (unsigned char*) qcow_realloc(*decompressed_data, (*decompressed_data_length + workspace.frame_buffer_len) * sizeof(unsigned char));
		if (*decompressed_data == NULL) {
			deallocate_workspace(&workspace);
			WARNING_LOG("Failed to qcow_reallocate the decompressed data buffer.\n");
			return -ZSTD_IO_ERROR;
		}

		mem_cpy(*decompressed_data + *decompressed_data_length, workspace.frame_buffer, workspace.frame_buffer_len);
		*decompressed_data_length += workspace.frame_buffer_len;
	}

	deallocate_workspace(&workspace);

	return ZSTD_NO_ERROR; 
}

/* ---------------------------------------------------------------------------------------------------------- */
// TODO: Rewrite the comments for better orienting external supporters
// TODO: Rewrite the warnings msgs to have a compact style
// TODO: Note that as most of CPUs use little-endian, and that the ZSTD format follows that, it could be unjustified the use of LE_CONVERT to convert to LE from BE, in rare cases.
unsigned char* zstd_inflate(unsigned char* stream, unsigned int size, unsigned int* decompressed_data_length, int* zstd_err) {
	unsigned char* decompressed_data = (unsigned char*) qcow_calloc(1, sizeof(unsigned char));
	if (decompressed_data == NULL) {
		QCOW_SAFE_FREE(stream);
		WARNING_LOG("Failed to allocate decompressed data buffer.\n");
		*zstd_err = -ZSTD_IO_ERROR;
		return ((unsigned char*) "An error occurred while initializing the buffer for the decompressed data.\n");
	}

	unsigned int frames_cnt = 0;
	BitStream bit_stream = CREATE_BIT_STREAM(stream, size);
	unsigned long int expected_decompression_size = (*decompressed_data_length > 0) ? *decompressed_data_length : 0x1FFFFFFFF;
	*decompressed_data_length = 0;
	do {
		DEBUG_LOG("Parsing frame num %u:\n", frames_cnt);
		if ((*zstd_err = parse_frames(&bit_stream, &decompressed_data, decompressed_data_length))) {
			QCOW_MULTI_FREE(stream, decompressed_data);
			return ((unsigned char*) "An error occurred while parsing the frame.\n");
		}
		frames_cnt++;
	} while (!IS_EOS(&bit_stream) && !bit_stream.error && *decompressed_data_length < expected_decompression_size);
	
	if (expected_decompression_size != 0x1FFFFFFFF && *decompressed_data_length != expected_decompression_size) {
		*zstd_err = -ZSTD_DECOMPRESSED_SIZE_MISMATCH;		
		return ((unsigned char*) "Decompressed size doesn't match the expected decompressed size.\n");
	}

	*zstd_err = 0;
	QCOW_SAFE_FREE(stream);

	return decompressed_data;
}

#endif //_ZSTD_H_
