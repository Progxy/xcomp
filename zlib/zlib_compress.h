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

#ifndef _ZLIB_COMPRESS_H_
#define _ZLIB_COMPRESS_H_

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Resources: deflate <https://www.ietf.org/rfc/rfc1951.txt> *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// TODO: Possibly create hf_tree struct to contain all the length - size couples, generally reduce the size of functions' declaration
// TODO: Write better comments and error messages

// -----------------
//  Constant Values
// -----------------
#define BLOCK_DELIMITER 256
#define MAX_HF_SIZE     288 
#define WINDOW_SIZE     0x7FFF // 32KB window

#define DEALLOCATE_TREES(...) 																	\
	do {																						\
		HFTree* hf_trees[] = { NULL, ##__VA_ARGS__ };							 				\
		for (unsigned int i = 1; i < XCOMP_ARR_SIZE(hf_trees); ++i) deallocate_hf_tree(hf_trees[i]);	\
	} while (FALSE)

#define CREATE_CALLBACK(...) \
	do {					 \
		__VA_ARGS__;		 \
	} while (FALSE)

/* -------------------------------------------------------------------------------------------------------- */
// ------------------
//  Static variables
// ------------------
static const unsigned short int fixed_hf_distances_table[] = {
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE,
	0xF, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A,
	0x1B, 0x1C, 0x1D, 0x1E, 0x1F
};

static const unsigned char fixed_hf_distances_lengths[] = {
	0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
	0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
	0x5, 0x5
};

static const unsigned short int fixed_hf_literals_table[] = {
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
	0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53,
	0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B,
	0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x80, 0x81, 0x82, 0x83,
	0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B,
	0x9C, 0x9D, 0x9E, 0x9F, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
	0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3,
	0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	0x190, 0x191, 0x192, 0x193, 0x194, 0x195, 0x196, 0x197, 0x198, 0x199,
	0x19A, 0x19B, 0x19C, 0x19D, 0x19E, 0x19F, 0x1A0, 0x1A1, 0x1A2, 0x1A3,
	0x1A4, 0x1A5, 0x1A6, 0x1A7, 0x1A8, 0x1A9, 0x1AA, 0x1AB, 0x1AC, 0x1AD,
	0x1AE, 0x1AF, 0x1B0, 0x1B1, 0x1B2, 0x1B3, 0x1B4, 0x1B5, 0x1B6, 0x1B7,
	0x1B8, 0x1B9, 0x1BA, 0x1BB, 0x1BC, 0x1BD, 0x1BE, 0x1BF, 0x1C0, 0x1C1,
	0x1C2, 0x1C3, 0x1C4, 0x1C5, 0x1C6, 0x1C7, 0x1C8, 0x1C9, 0x1CA, 0x1CB,
	0x1CC, 0x1CD, 0x1CE, 0x1CF, 0x1D0, 0x1D1, 0x1D2, 0x1D3, 0x1D4, 0x1D5,
	0x1D6, 0x1D7, 0x1D8, 0x1D9, 0x1DA, 0x1DB, 0x1DC, 0x1DD, 0x1DE, 0x1DF,
	0x1E0, 0x1E1, 0x1E2, 0x1E3, 0x1E4, 0x1E5, 0x1E6, 0x1E7, 0x1E8, 0x1E9,
	0x1EA, 0x1EB, 0x1EC, 0x1ED, 0x1EE, 0x1EF, 0x1F0, 0x1F1, 0x1F2, 0x1F3,
	0x1F4, 0x1F5, 0x1F6, 0x1F7, 0x1F8, 0x1F9, 0x1FA, 0x1FB, 0x1FC, 0x1FD,
	0x1FE, 0x1FF, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB,
	0xC, 0xD, 0xE, 0xF, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0xC0,
	0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7
};

static const unsigned char fixed_hf_literals_lengths[] = {
	0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
	0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
	0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
	0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
	0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
	0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
	0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
	0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
	0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
	0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9,
	0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9,
	0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9,
	0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9,
	0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9,
	0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9,
	0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9,
	0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9,
	0x9, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7,
	0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x8, 0x8, 0x8, 0x8, 0x8,
	0x8, 0x8, 0x8
};	

#define FIXED_LITERALS_TREE(hf) 								\
	hf.table = (unsigned short int*) fixed_hf_literals_table; 	\
	hf.lengths = (unsigned char*) fixed_hf_literals_lengths;	\
	hf.size = HF_LITERALS_SIZE;									\
	hf.is_fixed = TRUE;

#define FIXED_DISTANCE_TREE(hf) 								\
	hf.table = (unsigned short int*) fixed_hf_distances_table;	\
	hf.lengths = (unsigned char*) fixed_hf_distances_lengths;	\
	hf.size = HF_DISTANCE_SIZE;									\
	hf.is_fixed = TRUE;

/* -------------------------------------------------------------------------------------------------------- */
// ---------
//  Structs
// ---------
typedef struct Match {
	unsigned short int literal;
	unsigned char distance;
	unsigned char length_diff;
	unsigned short int distance_diff;
} Match;

typedef struct HFNode {
    unsigned short int symbol;
    unsigned int freq;
} HFNode;

typedef struct RLEStream {
	unsigned char value;
	unsigned char repeat_cnt;
} RLEStream;

typedef struct HFTree {
	unsigned char* lengths;
	unsigned short int* table;
	unsigned short int size;
	unsigned char is_fixed;
} HFTree;

/* -------------------------------------------------------------------------------------------------------- */
// ------------------------
//  Functions Declarations
// ------------------------
static int length_distance_encoding(const unsigned char* data_stream, unsigned int data_stream_size, Match** distance_encoding, unsigned int* distance_encoding_cnt);
static void update_hf_nodes(HFNode new_node, HFNode* hf_nodes, unsigned int hf_nodes_cnt);
static int build_hf_table(HFTree* hf_tree);
static int generate_hf_tree(unsigned short int* data_stream, unsigned int data_stream_size, HFTree* hf_tree);
static int rle_encoding(RLEStream** rle_encoded, unsigned short int* rle_encoded_size, HFTree hf_literals, HFTree hf_distances);
static int generate_hf_trees(Match* distance_encoded, unsigned int distance_encoded_size, BitStream* buffer, HFTree* hf_literals, HFTree* hf_distances);
static int hf_encode_block(HFTree hf_literals, HFTree hf_distances, Match* distance_encoding, unsigned int distance_encoding_cnt, BitStream* buffer);
static int encode_uncompressed_block(BitStream* compressed_bit_stream, unsigned char* data_buffer, unsigned int data_buffer_len, unsigned char is_final) ;
static int hf_compressed_block(BType method, BitStream* buffer, Match* distance_encoding, unsigned int distance_encoding_cnt, unsigned char is_final);
static int compress_block(BitStream* compressed_bit_stream, unsigned char* data_buffer, unsigned int data_buffer_len, unsigned char is_final);

/// NOTE: the stream will be always deallocated both in case of failure and success.
/// 	  Furthermore, the function allocates the returned stream of bytes, so that
/// 	  once it's on the hand of the caller, it's responsible to manage that memory.
unsigned char* zlib_deflate(unsigned char* data_buffer, unsigned int data_buffer_len, unsigned int* compressed_data_len, int* zlib_err);	

/* -------------------------------------------------------------------------------------------------------- */

static void deallocate_hf_tree(HFTree* hf_tree) {
	XCOMP_SAFE_FREE(hf_tree -> lengths);
	XCOMP_SAFE_FREE(hf_tree -> table);
	return;
}

static int length_distance_encoding(const unsigned char* data_stream, unsigned int data_stream_size, Match** distance_encoding, unsigned int* distance_encoding_cnt) {
	*distance_encoding_cnt = MIN(data_stream_size, 3);
	*distance_encoding = (Match*) xcomp_realloc(*distance_encoding, *distance_encoding_cnt * sizeof(Match));
	if (*distance_encoding == NULL) {
		WARNING_LOG("Failed to xcomp_reallocate buffer for distance_encoding.\n");
		return -ZLIB_IO_ERROR;
	}

	for (unsigned char i = 0; i < *distance_encoding_cnt; ++i) (*distance_encoding)[i].literal = data_stream[i];
	
	unsigned short int cur_len = 0;
	for (long int i = 3; i < data_stream_size; i += cur_len) {
		unsigned short int found = 0;
		cur_len = 3;
		
		for (int j = 0; (j < i - cur_len) && (i < data_stream_size - cur_len); ++j) {
			if (mem_n_cmp((const char*) (data_stream + i), (const char*) (data_stream + j), cur_len) == 0) {
				cur_len++;
				found = i - j;
				if (cur_len == 259) break;
				continue;	
			}
		}

		cur_len--;
		
		*distance_encoding = (Match*) xcomp_realloc(*distance_encoding, sizeof(Match) * (++(*distance_encoding_cnt)));
		if (*distance_encoding == NULL) {
			WARNING_LOG("Failed to xcomp_reallocate buffer for distance_encoding.\n");
			return -ZLIB_IO_ERROR;
		}
		
		(*distance_encoding)[*distance_encoding_cnt - 1] = (Match) {0};
	
		if (cur_len > 2) {
			const unsigned short int length_base_values[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
			unsigned char len_ind = 0;
			for (; len_ind < 29; ++len_ind) { 
				if (cur_len < length_base_values[len_ind]) {
					--len_ind;
					break;
				} else if (cur_len == length_base_values[len_ind]) break;
			}

			(*distance_encoding)[*distance_encoding_cnt - 1].length_diff = cur_len - length_base_values[len_ind];
			(*distance_encoding)[*distance_encoding_cnt - 1].literal = 257 + len_ind;
			
			const unsigned short int dist_base_values[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
			unsigned char dist_ind = 0;
			for (; dist_ind < 30; ++dist_ind) {
				if (found < dist_base_values[dist_ind]) {
					--dist_ind;
					break;
				} else if (found == dist_base_values[dist_ind]) break;
			}
			
			dist_ind = MIN(dist_ind, 29);

			(*distance_encoding)[*distance_encoding_cnt - 1].distance_diff = found - dist_base_values[dist_ind];
			(*distance_encoding)[*distance_encoding_cnt - 1].distance = dist_ind;
		} else {
			(*distance_encoding)[*distance_encoding_cnt - 1].literal = data_stream[i];
			cur_len = 1;
		}
	}
	
	// Append the block delimiter
	*distance_encoding = (Match*) xcomp_realloc(*distance_encoding, sizeof(Match) * (++(*distance_encoding_cnt)));
	if (*distance_encoding == NULL) {
		WARNING_LOG("Failed to xcomp_reallocate buffer for distance_encoding.\n");
		return -ZLIB_IO_ERROR;
	}
	
	(*distance_encoding)[*distance_encoding_cnt - 1] = (Match) {0};
	(*distance_encoding)[*distance_encoding_cnt - 1].literal = BLOCK_DELIMITER;

	return ZLIB_NO_ERROR;
}

static void update_hf_nodes(HFNode new_node, HFNode* hf_nodes, unsigned int hf_nodes_cnt) {
	for (unsigned short int i = 0; i < hf_nodes_cnt - 1; ++i) {
		if ((hf_nodes[i].freq > new_node.freq) || (hf_nodes[i].freq == new_node.freq && hf_nodes[i].symbol > new_node.symbol)) {
			// Move one to the left the others and insert it
			mem_move(hf_nodes + i + 1, hf_nodes + i, (hf_nodes_cnt - 1 - i) * sizeof(HFNode));
			hf_nodes[i] = new_node;	
			return;
		}
	}
	hf_nodes[hf_nodes_cnt - 1] = new_node;
	return;
}

static int build_hf_table(HFTree* hf_tree) {
	hf_tree -> table = (unsigned short int*) xcomp_calloc(hf_tree -> size, sizeof(unsigned short int));
	if (hf_tree -> table == NULL) {
		WARNING_LOG("Failed to allocate buffer for hf_tree.\n");
		return -ZLIB_IO_ERROR;
	}

	unsigned int bl_cnts[16] = {0};	
	for (unsigned short int i = 0; i < hf_tree -> size; ++i) bl_cnts[(hf_tree -> lengths)[i]]++;
	bl_cnts[0] = 0;
	
	unsigned short int mins[16] = {0};
	for (unsigned char i = 1; i < 16; ++i) mins[i] = (mins[i - 1] + bl_cnts[i - 1]) << 1;
	for (unsigned short int i = 0; i < hf_tree -> size; ++i) if ((hf_tree -> lengths)[i]) (hf_tree -> table)[i] = mins[(hf_tree -> lengths)[i]]++;

	return ZLIB_NO_ERROR;
}

// Compute Huffman code lengths from a frequency table
static int generate_hf_tree(unsigned short int* data_stream, unsigned int data_stream_size, HFTree* hf_tree) {
	unsigned int frequencies[MAX_HF_SIZE] = {0};
	for (unsigned int i = 0; i < data_stream_size; ++i) frequencies[data_stream[i]]++;

	HFNode hf_nodes[MAX_HF_SIZE] = {0};  // Heap for building the tree
	unsigned short int hf_nodes_cnt = 0;
	for (unsigned short int i = 0; i < MAX_HF_SIZE; ++i) {
		if (frequencies[i]) {
			HFNode new_node = (HFNode) { .symbol = i, .freq = frequencies[i] };
			update_hf_nodes(new_node, hf_nodes, ++hf_nodes_cnt);
			hf_tree -> size = MAX(hf_tree -> size, i + 1);
		}
	}
	
	if (hf_tree -> size == 0) {
		hf_tree -> table = (unsigned short int*) xcomp_calloc(MAX(hf_tree -> size, 1), sizeof(unsigned short int));
		hf_tree -> lengths = (unsigned char*) xcomp_calloc(MAX(hf_tree -> size, 1), sizeof(unsigned char));
		return ZLIB_NO_ERROR;
	}

    // Build Huffman tree (iterative method)
	unsigned int parent_size = hf_tree -> size;
	unsigned int* parent = (unsigned int*) xcomp_calloc(parent_size, sizeof(unsigned int));
	if (parent == NULL) {
		WARNING_LOG("Failed to allocate buffer for parent.\n");
		return -ZLIB_IO_ERROR;
	}

    while (hf_nodes_cnt > 1) {
        // Take two smallest nodes
        HFNode left = hf_nodes[0];
        HFNode right = hf_nodes[1];

        // Remove them from heap
        mem_move(hf_nodes, hf_nodes + 2, (hf_nodes_cnt - 2) * sizeof(HFNode));
        hf_nodes_cnt--;

        // Create a new merged node
        HFNode new_node = (HFNode) { .symbol = parent_size++, .freq = left.freq + right.freq};
		parent = (unsigned int*) xcomp_realloc(parent, sizeof(unsigned int) * parent_size);
		if (parent == NULL) {
			WARNING_LOG("Failed to xcomp_reallocate buffer for parent.\n");
			return -ZLIB_IO_ERROR;
		}

		parent[parent_size - 1] = 0;

        // Assign parents for tree traversal
        parent[left.symbol] = new_node.symbol;
        parent[right.symbol] = new_node.symbol;

		update_hf_nodes(new_node, hf_nodes, hf_nodes_cnt);
    }

    // Assign bit-lengths from depths
    hf_tree -> lengths = (unsigned char*) xcomp_calloc(hf_tree -> size, sizeof(unsigned char));
	if (hf_tree -> lengths == NULL) {
		WARNING_LOG("Failed to allocate buffer for hf_lengths.\n");
		return -ZLIB_IO_ERROR;
	}
	
	for (unsigned short int i = 0; i < hf_tree -> size; i++) {
        unsigned int node = i;
        while (parent[node]) {
			((hf_tree -> lengths)[i])++;
            node = parent[node];
        }
    }

	XCOMP_SAFE_FREE(parent);

	int err = 0;
	if ((err = build_hf_table(hf_tree)) < 0) {
		XCOMP_SAFE_FREE(hf_tree -> lengths);
		WARNING_LOG("An error occurred while building the hf table.\n");
		return err;
	}

	return ZLIB_NO_ERROR;
}

static int rle_encoding(RLEStream** rle_encoded, unsigned short int* rle_encoded_size, HFTree hf_literals, HFTree hf_distances) {
	*rle_encoded = (RLEStream*) xcomp_calloc(hf_literals.size + hf_distances.size, sizeof(RLEStream));
	if (*rle_encoded == NULL) {
		WARNING_LOG("Failed to allocate buffer for rle_encoded.\n");
		return -ZLIB_IO_ERROR;
	}
	
	unsigned char previous_code = (hf_literals.lengths)[0];
	*rle_encoded_size = 0;
	(*rle_encoded)[(*rle_encoded_size)++].value = (hf_literals.lengths)[0];

	unsigned char zero_cnt = 0;
	unsigned char repeat_cnt = 0;
	for (unsigned short int i = 1; i < hf_literals.size; ++i) {
		if ((hf_literals.lengths)[i] == 0) {
			for (zero_cnt = 1; zero_cnt < 138; ++i, ++zero_cnt) if (i + 1 >= hf_literals.size || (hf_literals.lengths)[i + 1] != 0) break;
			if (zero_cnt < 138 && i == hf_literals.size) break;
			else if (zero_cnt == 2) (*rle_encoded)[(*rle_encoded_size)++].value = (hf_literals.lengths)[i - 1];
		} else if ((hf_literals.lengths)[i] == previous_code) {
			for (repeat_cnt = 1; repeat_cnt < 6; ++i, ++repeat_cnt) if (i + 1 >= hf_literals.size || (hf_literals.lengths)[i + 1] != previous_code) break;
			if (repeat_cnt < 6 && i == hf_literals.size) break;
			else if (repeat_cnt == 2) (*rle_encoded)[(*rle_encoded_size)++].value = (hf_literals.lengths)[i - 1];
		}
		
		if (repeat_cnt >= 3) (*rle_encoded)[*rle_encoded_size].repeat_cnt = repeat_cnt, (*rle_encoded)[*rle_encoded_size].value = 16; 
		else if (zero_cnt >= 3) (*rle_encoded)[*rle_encoded_size].repeat_cnt = zero_cnt, (*rle_encoded)[*rle_encoded_size].value = 17 + (zero_cnt > 10), previous_code = 0;
		else (*rle_encoded)[*rle_encoded_size].value = (hf_literals.lengths)[i], previous_code = (hf_literals.lengths)[i];
		(*rle_encoded_size)++;
		repeat_cnt = 0, zero_cnt = 0;
	}

	if ((repeat_cnt || zero_cnt) && (previous_code != (hf_distances.lengths)[0])) {
		if (repeat_cnt >= 3) (*rle_encoded)[*rle_encoded_size].repeat_cnt = repeat_cnt, (*rle_encoded)[(*rle_encoded_size)++].value = 16; 
		else if (zero_cnt >= 3) (*rle_encoded)[*rle_encoded_size].repeat_cnt = zero_cnt, (*rle_encoded)[(*rle_encoded_size)++].value = 17 + (zero_cnt > 10), previous_code = 0;
		else {
			unsigned char limit = MAX(repeat_cnt, zero_cnt);
			if (zero_cnt) previous_code = 0;
			for (unsigned char i = 0; i < limit; ++i, ++(*rle_encoded_size)) (*rle_encoded)[*rle_encoded_size].repeat_cnt = 0, (*rle_encoded)[*rle_encoded_size].value = previous_code;
		}
	} else if ((repeat_cnt == 1 || zero_cnt == 1) && (previous_code == (hf_distances.lengths)[0] && previous_code != (hf_distances.lengths)[1])) {
		if (zero_cnt) previous_code = 0;
		(*rle_encoded)[*rle_encoded_size].repeat_cnt = 0, (*rle_encoded)[(*rle_encoded_size)++].value = previous_code;
	}
	
	for (unsigned short int i = 0; i < hf_distances.size; ++i) {
		if ((hf_distances.lengths)[i] == 0) {
			for (zero_cnt = 1; zero_cnt < 138; ++i, ++zero_cnt) if (i + 1 >= hf_distances.size || (hf_distances.lengths)[i + 1] != 0) break;
			if (zero_cnt < 138 && i == hf_distances.size) break;
			else if (zero_cnt == 2) (*rle_encoded)[(*rle_encoded_size)++].value = (hf_distances.lengths)[i - 1];
		} else if ((hf_distances.lengths)[i] == previous_code) {
			for (repeat_cnt = 1; repeat_cnt < 6; ++i, ++repeat_cnt) if (i + 1 >= hf_distances.size || (hf_distances.lengths)[i + 1] != previous_code) break;
			if (repeat_cnt < 6 && i == hf_distances.size) break;
			else if (repeat_cnt == 2) (*rle_encoded)[(*rle_encoded_size)++].value = (hf_distances.lengths)[i - 1];
		}
		
		if (repeat_cnt >= 3) (*rle_encoded)[*rle_encoded_size].repeat_cnt = repeat_cnt, (*rle_encoded)[*rle_encoded_size].value = 16; 
		else if (zero_cnt >= 3) (*rle_encoded)[*rle_encoded_size].repeat_cnt = zero_cnt, (*rle_encoded)[*rle_encoded_size].value = 17 + (zero_cnt > 10), previous_code = 0;
		else (*rle_encoded)[*rle_encoded_size].repeat_cnt = 0, (*rle_encoded)[*rle_encoded_size].value = (hf_distances.lengths)[i], previous_code = (hf_distances.lengths)[i];
		(*rle_encoded_size)++;
		repeat_cnt = 0, zero_cnt = 0;
	}

	if (repeat_cnt >= 3) (*rle_encoded)[*rle_encoded_size].repeat_cnt = repeat_cnt, (*rle_encoded)[(*rle_encoded_size)++].value = 16; 
	else if (zero_cnt >= 3) (*rle_encoded)[*rle_encoded_size].repeat_cnt = zero_cnt, (*rle_encoded)[(*rle_encoded_size)++].value = 17 + (zero_cnt > 10), previous_code = 0;
	else {
		unsigned char limit = MAX(repeat_cnt, zero_cnt);
		if (zero_cnt) previous_code = 0;
		for (unsigned char i = 0; i < limit; ++i, ++(*rle_encoded_size)) (*rle_encoded)[*rle_encoded_size].repeat_cnt = 0, (*rle_encoded)[*rle_encoded_size].value = previous_code;
	}

	*rle_encoded = (RLEStream*) xcomp_realloc(*rle_encoded, (*rle_encoded_size) * sizeof(RLEStream));
	if (*rle_encoded == NULL) {
		WARNING_LOG("Failed to xcomp_reallocate buffer for rle_encoded.\n");
		return -ZLIB_IO_ERROR;
	}

	return ZLIB_NO_ERROR;
}

static int generate_hf_trees(Match* distance_encoded, unsigned int distance_encoded_size, BitStream* buffer, HFTree* hf_literals, HFTree* hf_distances) {
	unsigned int distance_size = 0;
	unsigned short int* literals_data = (unsigned short int*) xcomp_calloc(distance_encoded_size, sizeof(unsigned short int));
	if (literals_data == NULL) {
		WARNING_LOG("Failed to allocate buffer for literals_data.\n");
		return -ZLIB_IO_ERROR;
	}

	unsigned short int* distance_data = (unsigned short int*) xcomp_calloc(distance_encoded_size, sizeof(unsigned short int));
	if (distance_data == NULL) {
		XCOMP_SAFE_FREE(literals_data);
		WARNING_LOG("Failed to allocate buffer for distance_data.\n");
		return -ZLIB_IO_ERROR;
	}

	for (unsigned int i = 0; i < distance_encoded_size; ++i) {
		literals_data[i] = distance_encoded[i].literal;
		if (literals_data[i] > 256) distance_data[distance_size++] = distance_encoded[i].distance;
	}

	if (distance_size == 0) XCOMP_SAFE_FREE(distance_data);
	else {
		distance_data = (unsigned short int*) xcomp_realloc(distance_data, distance_size * sizeof(unsigned short int));
		if (distance_data == NULL) {
			XCOMP_SAFE_FREE(literals_data);
			WARNING_LOG("Failed to xcomp_reallocate buffer for distance_data.\n");
			return -ZLIB_IO_ERROR;
		}
	}

	// Generate the tables
	int err = 0;
	if ((err = generate_hf_tree(literals_data, distance_encoded_size, hf_literals)) < 0) {
		XCOMP_MULTI_FREE(literals_data, distance_data);
		WARNING_LOG("An error occurred while generating the hf_tree for literals.\n");
		return err;
	}

	if ((err = generate_hf_tree(distance_data, distance_size, hf_distances)) < 0) {
		deallocate_hf_tree(hf_literals);
		XCOMP_MULTI_FREE(literals_data, distance_data);
		WARNING_LOG("An error occurred while generating the hf_tree for distances.\n");
		return err;
	}
	
	XCOMP_MULTI_FREE(literals_data, distance_data);
	
	RLEStream* rle_encoded = NULL;
	unsigned short int rle_encoded_size = 0;
	if ((err = rle_encoding(&rle_encoded, &rle_encoded_size, *hf_literals, *hf_distances)) < 0) {
		DEALLOCATE_TREES(hf_literals, hf_distances);
		WARNING_LOG("An error occurred while rle_encoding.\n");
		return err;
	}

	unsigned short int* rle_encoded_data = (unsigned short int*) xcomp_calloc(rle_encoded_size, sizeof(unsigned short int));
	if (rle_encoded_data == NULL) {
		DEALLOCATE_TREES(hf_literals, hf_distances);
		XCOMP_SAFE_FREE(rle_encoded);
		WARNING_LOG("Failed to allocate buffer for rle_encoded_data.\n");
		return -ZLIB_IO_ERROR;
	}

	for (unsigned short int i = 0; i < rle_encoded_size; ++i) rle_encoded_data[i] = rle_encoded[i].value;

	HFTree hf_tree = { .size = HF_TABLE_SIZE };
	if ((err = generate_hf_tree(rle_encoded_data, rle_encoded_size, &hf_tree)) < 0) {
		DEALLOCATE_TREES(hf_literals, hf_distances);
		XCOMP_MULTI_FREE(rle_encoded, rle_encoded_data);
		WARNING_LOG("An error occurred while generating the hf_tree for the previous hf.\n");
		return err;
	}

	XCOMP_SAFE_FREE(rle_encoded_data);

	SAFE_BIT_WRITE(buffer, MAX(257, hf_literals -> size) - 257, 5, CREATE_CALLBACK(DEALLOCATE_TREES(&hf_tree, hf_literals, hf_distances); XCOMP_SAFE_FREE(rle_encoded)));
	SAFE_BIT_WRITE(buffer, MAX(1, hf_distances -> size) - 1, 5, CREATE_CALLBACK(DEALLOCATE_TREES(&hf_tree, hf_literals, hf_distances); XCOMP_SAFE_FREE(rle_encoded)));

	const unsigned char order_of_code_lengths[] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
	unsigned char order_size = 18;
	for (; order_size > 3; --order_size) if ((hf_tree.lengths)[order_of_code_lengths[order_size]] > 0) break;
	SAFE_BIT_WRITE(buffer, (order_size + 1) - 4, 4, CREATE_CALLBACK(DEALLOCATE_TREES(&hf_tree, hf_literals, hf_distances); XCOMP_SAFE_FREE(rle_encoded)));

	for (unsigned char i = 0; i <= order_size; ++i) SAFE_BIT_WRITE(buffer, (hf_tree.lengths)[order_of_code_lengths[i]], 3, CREATE_CALLBACK(DEALLOCATE_TREES(&hf_tree, hf_literals, hf_distances); XCOMP_SAFE_FREE(rle_encoded)));

	for (unsigned short int i = 0; i < rle_encoded_size; ++i) {
		unsigned char value = rle_encoded[i].value;
		unsigned char repeat_cnt = rle_encoded[i].repeat_cnt;
		SAFE_REV_BIT_WRITE(buffer, (hf_tree.table)[value], (hf_tree.lengths)[value], CREATE_CALLBACK(DEALLOCATE_TREES(&hf_tree, hf_literals, hf_distances); XCOMP_SAFE_FREE(rle_encoded)));
		if (repeat_cnt) {
			if (value == 16) SAFE_BIT_WRITE(buffer, repeat_cnt - 3, 2, CREATE_CALLBACK(DEALLOCATE_TREES(&hf_tree, hf_literals, hf_distances); XCOMP_SAFE_FREE(rle_encoded)));
			else if (value == 17) SAFE_BIT_WRITE(buffer, repeat_cnt - 3, 3,CREATE_CALLBACK(DEALLOCATE_TREES(&hf_tree, hf_literals, hf_distances); XCOMP_SAFE_FREE(rle_encoded)));
			else SAFE_BIT_WRITE(buffer, repeat_cnt - 11, 7, CREATE_CALLBACK(DEALLOCATE_TREES(&hf_tree, hf_literals, hf_distances); XCOMP_SAFE_FREE(rle_encoded)));
		} 
	}

	deallocate_hf_tree(&hf_tree);
	XCOMP_SAFE_FREE(rle_encoded);

	return ZLIB_NO_ERROR;
}

static int hf_encode_block(HFTree hf_literals, HFTree hf_distances, Match* distance_encoding, unsigned int distance_encoding_cnt, BitStream* buffer) {
    const unsigned char lenghts_extra_bits[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    const unsigned char distances_extra_bits[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

	for (unsigned int i = 0; i < distance_encoding_cnt; ++i) {
		unsigned short int literal = distance_encoding[i].literal;
		SAFE_REV_BIT_WRITE(buffer, (hf_literals.table)[literal], (hf_literals.lengths)[literal]);
		if (literal > 256) {
			unsigned char distance = distance_encoding[i].distance;
			SAFE_BIT_WRITE(buffer, distance_encoding[i].length_diff, lenghts_extra_bits[literal - 257]);
			SAFE_REV_BIT_WRITE(buffer, (hf_distances.table)[distance], (hf_distances.lengths)[distance]);
			SAFE_BIT_WRITE(buffer, distance_encoding[i].distance_diff, distances_extra_bits[distance]);
		}
	}

	return ZLIB_NO_ERROR;
}

static int encode_uncompressed_block(BitStream* compressed_bit_stream, unsigned char* data_buffer, unsigned int data_buffer_len, unsigned char is_final) {	
	SAFE_BIT_WRITE(compressed_bit_stream, is_final, 3);
	
	unsigned short int buffer_len = data_buffer_len & 0xFFFF;
	XCOMP_BE_CONVERT(&buffer_len, sizeof(unsigned short int));
	
	SAFE_BYTE_WRITE(compressed_bit_stream, sizeof(unsigned short int), 1, &buffer_len);
	buffer_len = ~buffer_len;
	SAFE_BYTE_WRITE(compressed_bit_stream, sizeof(unsigned short int), 1, &buffer_len);
	
	XCOMP_BE_CONVERT(data_buffer, data_buffer_len);
	SAFE_BYTE_WRITE(compressed_bit_stream, sizeof(unsigned char), data_buffer_len, data_buffer);
	
	return ZLIB_NO_ERROR;
}

static int hf_compressed_block(BType method, BitStream* buffer, Match* distance_encoding, unsigned int distance_encoding_cnt, unsigned char is_final) {
	int err = 0;

	// Calculate the Huffman Tree/Table
	SAFE_NEXT_BIT_WRITE(buffer, is_final, distance_encoding);         		
	SAFE_BIT_WRITE(buffer, method, 2, XCOMP_SAFE_FREE(distance_encoding));			

	HFTree hf_literals = {0};
	HFTree hf_distances = {0};
	if (method == COMPRESSED_DYNAMIC_HF) {
		if ((err = generate_hf_trees(distance_encoding, distance_encoding_cnt, buffer, &hf_literals, &hf_distances)) < 0) {
			WARNING_LOG("An error occurred while generating hf_tables.\n");
			return err;
		}
	} else {
		FIXED_LITERALS_TREE(hf_literals);
		FIXED_DISTANCE_TREE(hf_distances);
	}

	// Huffman encode the block, encapsulating it into a DEFLATE block (append block header, encoded data plus the encoded '256' to signal the end of the block)
	if ((err = hf_encode_block(hf_literals, hf_distances, distance_encoding, distance_encoding_cnt, buffer)) < 0) {
		if (!hf_literals.is_fixed)  DEALLOCATE_TREES(&hf_literals, &hf_distances);
		WARNING_LOG("An error occurred while encoding the block.\n");
		return err;
	} 

	if (!hf_literals.is_fixed) DEALLOCATE_TREES(&hf_literals, &hf_distances);

	return ZLIB_NO_ERROR;
}

static int compress_block(BitStream* compressed_bit_stream, unsigned char* data_buffer, unsigned int data_buffer_len, unsigned char is_final) {
	int err = 0;
	Match* distance_encoding = NULL;
	unsigned int distance_encoding_cnt = 0;
	if ((err = length_distance_encoding(data_buffer, data_buffer_len, &distance_encoding, &distance_encoding_cnt)) < 0) {
		WARNING_LOG("An error occurred while performing the length-distance encoding.\n");
		return err; 
	}
	
	// Static compression
	BitStream fixed_block_bit_stream = CREATE_BIT_STREAM(NULL, 0);
	if ((err = hf_compressed_block(COMPRESSED_FIXED_HF, &fixed_block_bit_stream, distance_encoding, distance_encoding_cnt, is_final)) < 0) {
		XCOMP_SAFE_FREE(distance_encoding);
		WARNING_LOG("An error occurred while compressing the block using FIXED_HF.\n");
		return err;
	}
	
	// Dynamic compression
	BitStream dynamic_block_bit_stream = CREATE_BIT_STREAM(NULL, 0);
	if ((err = hf_compressed_block(COMPRESSED_DYNAMIC_HF, &dynamic_block_bit_stream, distance_encoding, distance_encoding_cnt, is_final)) < 0) {
		XCOMP_SAFE_FREE(distance_encoding);
		deallocate_bit_stream(&fixed_block_bit_stream);
		WARNING_LOG("An error occurred while compressing the block using DYNAMIC_HF.\n");
		return err;
	}

	XCOMP_SAFE_FREE(distance_encoding);
	
	// Fallback no compression
	if (fixed_block_bit_stream.size > data_buffer_len + 5 && dynamic_block_bit_stream.size > data_buffer_len + 5) {
		deallocate_bit_stream(&fixed_block_bit_stream);
		deallocate_bit_stream(&dynamic_block_bit_stream);
		printf("compression_method: '%s', decompressed_size: %u\n", btypes_str[NO_COMPRESSION], data_buffer_len);
		if ((err = encode_uncompressed_block(compressed_bit_stream, data_buffer, data_buffer_len, is_final)) < 0) {
			WARNING_LOG("An error occurred while encoding the uncompressed block.\n");
			return err;
		}
		return ZLIB_NO_ERROR;
	}

	unsigned char is_fixed_better = fixed_block_bit_stream.size <= dynamic_block_bit_stream.size;
	printf("compression_method: '%s', decompressed_size: %u\n", btypes_str[is_fixed_better ? COMPRESSED_FIXED_HF : COMPRESSED_DYNAMIC_HF], data_buffer_len);
	BitStream block_bit_stream = is_fixed_better ? fixed_block_bit_stream : dynamic_block_bit_stream;
	if (is_fixed_better) deallocate_bit_stream(&dynamic_block_bit_stream);
	else deallocate_bit_stream(&fixed_block_bit_stream);
	
	bitstream_bit_copy(compressed_bit_stream, &block_bit_stream);
	if (compressed_bit_stream -> error) {
		if (!is_fixed_better) deallocate_bit_stream(&dynamic_block_bit_stream);
		else deallocate_bit_stream(&fixed_block_bit_stream);
		WARNING_LOG("Failed to bit copy the block bitstream into the compressed bitstream.\n");
		return -ZLIB_IO_ERROR;
	}

	// Deallocate the one the bit_stream actually used
	if (!is_fixed_better) deallocate_bit_stream(&dynamic_block_bit_stream);
	else deallocate_bit_stream(&fixed_block_bit_stream);

	return ZLIB_NO_ERROR;
}

unsigned char* zlib_deflate(unsigned char* data_buffer, unsigned int data_buffer_len, unsigned int* compressed_data_len, int* zlib_err) {
	*compressed_data_len = 0;
	BitStream compressed_bit_stream = CREATE_BIT_STREAM(NULL, 0);
	*zlib_err = -ZLIB_NO_ERROR;
	
	// Fragment the data in block of WINDOW_SIZE
	unsigned int buffer_offset = 0;
#ifdef _DEBUG
	unsigned int block_cnt = 0;
#endif //_DEBUG
	while (data_buffer_len >= WINDOW_SIZE) {
		DEBUG_LOG("Block %u: is_final: %u, ", ++block_cnt, data_buffer_len == WINDOW_SIZE);
		if ((*zlib_err = compress_block(&compressed_bit_stream, data_buffer + buffer_offset, WINDOW_SIZE, data_buffer_len == WINDOW_SIZE)) < 0) {
			XCOMP_SAFE_FREE(data_buffer);
			deallocate_bit_stream(&compressed_bit_stream);
			return ((unsigned char*) "An error occurred while compressing the block.\n");
		}
		data_buffer_len -= WINDOW_SIZE;
		buffer_offset += WINDOW_SIZE;
	}
	
	if (data_buffer_len > 0) {
		DEBUG_LOG("Block %u: is_final: 1, ", ++block_cnt);
		if ((*zlib_err = compress_block(&compressed_bit_stream, data_buffer + buffer_offset, data_buffer_len, TRUE)) < 0) {
			XCOMP_SAFE_FREE(data_buffer);
			deallocate_bit_stream(&compressed_bit_stream);
			return ((unsigned char*) "An error occurred while compressing the block.\n");
		}
	}

	XCOMP_SAFE_FREE(data_buffer);
	
	*compressed_data_len = compressed_bit_stream.size;
	return compressed_bit_stream.stream;
}

#endif
