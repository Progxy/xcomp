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

// TODO: Possibly create hf_tree struct to contain all the length - size
//       couples, generally reduce the size of functions' declaration
// TODO: Write better comments and error messages

/* -------------------------------------------------------------------------------------------------------- */
// ------------------
//  Static variables
// ------------------
static const unsigned short int fhf_dist_values[] = {
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE,
	0xF, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A,
	0x1B, 0x1C, 0x1D, 0x1E, 0x1F
};

static const unsigned char fhf_dist_lengths[] = {
	0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
	0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
	0x5, 0x5
};

static const unsigned short int fhf_lit_values[] = {
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

static const unsigned char fhf_lit_lengths[] = {
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

/* -------------------------------------------------------------------------------------------------------- */
// ---------
//  Structs
// ---------
typedef struct {
	unsigned short int* literal;
	unsigned char*      distance;
	unsigned char*      length_diff;
	unsigned short int* distance_diff;
	unsigned int cnt;
} Matches;

typedef struct {
	unsigned char* values;
	unsigned char* repeat_cnts;
	unsigned int cnt;
} RLEStream;

typedef struct {
    unsigned short int symbol;
    unsigned int freq;
} HFNode;

typedef struct {
	unsigned short int* values;
	unsigned char* lengths;
	unsigned short int size;
} HFTree;

/* -------------------------------------------------------------------------------------------------------- */
// ------------------------
//  Functions Declarations
// ------------------------
/// NOTE: the stream will be always deallocated both in case of failure and success.
/// 	  Furthermore, the function allocates the returned stream of bytes, so that
/// 	  once it's on the hand of the caller, it's responsible to manage that memory.
unsigned char* deflate_deflate(unsigned char* data_buffer, unsigned int data_buffer_len, unsigned int* compressed_data_len, int* zlib_err);	
unsigned char* zlib_deflate(unsigned char* data, unsigned int data_len, unsigned int* compressed_data_len, zlib_header_t* zlib_header, int* zlib_err);

static void deallocate_hf_tree(HFTree* hf_tree);

/* -------------------------------------------------------------------------------------------------------- */

static void encode_length_distance(zlib_buffer_t* buffer, Matches* distance_encoding, const unsigned int window_size) {
	const unsigned short int length_base_values[29]   = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
	const unsigned short int distance_base_values[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
	const unsigned int window_end = MIN(window_size, buffer -> size - buffer -> pos) + buffer -> pos;
	const unsigned int buffer_start = buffer -> pos;

	while (buffer -> pos < window_end) {
		unsigned int distance = buffer -> pos - buffer_start;
		unsigned int cur_distance = distance;
		unsigned int length = 0;
		unsigned char len_ind  = 0;
		unsigned char dist_ind = 29;
		while ((length < 259) && (length < distance) && (buffer -> pos + length < window_end)) {
			int is_diff = mem_n_cmp((const char*) (buffer -> data + buffer -> pos - distance), (const char*) (buffer -> data + buffer -> pos), length + 1);
			if (is_diff == 0) {
				cur_distance = distance;
				len_ind += (++length) > length_base_values[len_ind];
				while (cur_distance < distance_base_values[dist_ind]) dist_ind--;
				continue;
			}
			distance--;
		}
		
		if (length < 3) {
			// store the current byte as a literal if no backreference was found
			(distance_encoding -> literal)[(distance_encoding -> cnt)++] = (buffer -> data)[buffer -> pos++];
			continue;
		}

		// store length and distance if backreference was found
		(distance_encoding -> length_diff)[distance_encoding -> cnt]   = length - length_base_values[len_ind];
		(distance_encoding -> literal)[distance_encoding -> cnt]       = 257 + len_ind;
		(distance_encoding -> distance_diff)[distance_encoding -> cnt] = cur_distance - distance_base_values[dist_ind];
		(distance_encoding -> distance)[distance_encoding -> cnt]      = dist_ind;
		(distance_encoding -> cnt)++;
		buffer -> pos += length;
	}
	
	// Append the block delimiter
	(distance_encoding -> literal)[(distance_encoding -> cnt)++] = BLOCK_DELIMITER;
	
	return;
}

static int length_distance_encoding(zlib_buffer_t* buffer, Matches* distance_encoding, const unsigned int window_size, int* zlib_err) {
	const unsigned int size = MIN(window_size, buffer -> size - buffer -> pos);
	distance_encoding -> literal       = xcomp_calloc(size + 1, sizeof(unsigned short int));
	distance_encoding -> distance      = xcomp_calloc(size + 1, sizeof(unsigned char));
	distance_encoding -> length_diff   = xcomp_calloc(size + 1, sizeof(unsigned char));
	distance_encoding -> distance_diff = xcomp_calloc(size + 1, sizeof(unsigned short int));
	
	if (distance_encoding -> literal       == NULL || distance_encoding -> distance      == NULL ||
		distance_encoding -> length_diff   == NULL || distance_encoding -> distance_diff == NULL) {
		XCOMP_SAFE_FREE(distance_encoding -> literal);
		XCOMP_SAFE_FREE(distance_encoding -> distance);
		XCOMP_SAFE_FREE(distance_encoding -> length_diff);
		XCOMP_SAFE_FREE(distance_encoding -> distance_diff);
		WARNING_LOG("Failed to xcomp_reallocate buffer for distance_encoding.\n");
		*zlib_err = -ZLIB_IO_ERROR;
		return *zlib_err;
	}

	distance_encoding -> cnt = 0;
	encode_length_distance(buffer, distance_encoding, window_size);

	distance_encoding -> literal       = xcomp_realloc(distance_encoding -> literal, (distance_encoding -> cnt) * sizeof(unsigned short int));
	distance_encoding -> distance      = xcomp_realloc(distance_encoding -> distance, (distance_encoding -> cnt) * sizeof(unsigned char));
	distance_encoding -> length_diff   = xcomp_realloc(distance_encoding -> length_diff, (distance_encoding -> cnt) * sizeof(unsigned char));
	distance_encoding -> distance_diff = xcomp_realloc(distance_encoding -> distance_diff, (distance_encoding -> cnt) * sizeof(unsigned short int));
	if (distance_encoding -> literal       == NULL || distance_encoding -> distance      == NULL ||
		distance_encoding -> length_diff   == NULL || distance_encoding -> distance_diff == NULL) {
		XCOMP_SAFE_FREE(distance_encoding -> literal);
		XCOMP_SAFE_FREE(distance_encoding -> distance);
		XCOMP_SAFE_FREE(distance_encoding -> length_diff);
		XCOMP_SAFE_FREE(distance_encoding -> distance_diff);
		WARNING_LOG("Failed to xcomp_reallocate buffer for distance_encoding.\n");
		*zlib_err = -ZLIB_IO_ERROR;
		return *zlib_err;
	}

	return ZLIB_NO_ERROR;
}

static int get_next_code(const unsigned int index, const HFTree hf_literals, const HFTree hf_distances) {
	const unsigned int lengths_cnt = hf_literals.size + hf_distances.size;
	if (index + 1 < hf_literals.size) return (hf_literals.lengths)[index + 1];
	else if (index + 1 < lengths_cnt) return (hf_distances.lengths)[index + 1 - hf_literals.size];
	return -1;
}

static void encode_rle(RLEStream* rle_encoded, const HFTree hf_literals, const HFTree hf_distances) {
	rle_encoded -> cnt = 0;
	
	const unsigned int lengths_cnt = hf_literals.size + hf_distances.size;
	int curr_code = get_next_code(-1, hf_literals, hf_distances);
	unsigned char repeat_cnt = 1;

	for (unsigned int i = 0; i <= lengths_cnt; ++i) {
		int next_code = get_next_code(i, hf_literals, hf_distances);
		while (next_code == curr_code && next_code != -1) {
			repeat_cnt++;
			i++;
			next_code = get_next_code(i, hf_literals, hf_distances);
		}

		(rle_encoded -> repeat_cnts)[rle_encoded -> cnt] = repeat_cnt;
		(rle_encoded -> values)[(rle_encoded -> cnt)++]  = curr_code; 
		
		repeat_cnt = 1; 
		curr_code = next_code;
		if (curr_code == -1) break;
	}

	return;
}

static int rle_encoding(RLEStream* rle_encoded, HFTree hf_literals, HFTree hf_distances, int* zlib_err) {
	rle_encoded -> values      = (unsigned char*) xcomp_calloc(hf_literals.size + hf_distances.size, sizeof(unsigned char));
	rle_encoded -> repeat_cnts = (unsigned char*) xcomp_calloc(hf_literals.size + hf_distances.size, sizeof(unsigned char));
	if (rle_encoded -> values == NULL || rle_encoded -> repeat_cnts == NULL) {
		XCOMP_MULTI_FREE(rle_encoded -> values, rle_encoded -> repeat_cnts);
		WARNING_LOG("Failed to allocate buffer for rle_encoded.\n");
		*zlib_err = -ZLIB_IO_ERROR;
		return *zlib_err;
	}
	
	encode_rle(rle_encoded, hf_literals, hf_distances);

	rle_encoded -> values      = (unsigned char*) xcomp_realloc(rle_encoded -> values, (rle_encoded -> cnt) * sizeof(unsigned char));
	rle_encoded -> repeat_cnts = (unsigned char*) xcomp_realloc(rle_encoded -> repeat_cnts, (rle_encoded -> cnt) * sizeof(unsigned char));
	if (rle_encoded -> values == NULL || rle_encoded -> repeat_cnts == NULL) {
		XCOMP_MULTI_FREE(rle_encoded -> values, rle_encoded -> repeat_cnts);
		WARNING_LOG("Failed to xcomp_reallocate buffer for rle_encoded.\n");
		*zlib_err = -ZLIB_IO_ERROR;
		return *zlib_err;
	}

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

static void build_hf_tree(HFTree* hf_tree) {
	unsigned int bl_cnts[16] = {0};	
	for (unsigned short int i = 0; i < hf_tree -> size; ++i) bl_cnts[(hf_tree -> lengths)[i]]++;
	bl_cnts[0] = 0;
	
	unsigned short int mins[16] = {0};
	for (unsigned char i = 1; i < 16; ++i) mins[i] = (mins[i - 1] + bl_cnts[i - 1]) << 1;
	for (unsigned short int i = 0; i < hf_tree -> size; ++i) if ((hf_tree -> lengths)[i]) (hf_tree -> values)[i] = mins[(hf_tree -> lengths)[i]]++;

	return;
}

static int compute_hf_codes(HFNode* hf_nodes, unsigned int hf_nodes_cnt, HFTree* hf_tree, int* zlib_err) {
    // Build Huffman tree (iterative method)
	unsigned int parent_size = hf_tree -> size;
	unsigned int* parent = (unsigned int*) xcomp_calloc(parent_size, sizeof(unsigned int));
	if (parent == NULL) {
		WARNING_LOG("Failed to allocate buffer for parent.\n");
		*zlib_err = -ZLIB_IO_ERROR;
		return *zlib_err;
	}

	if (hf_nodes_cnt == 1) parent[hf_nodes -> symbol] = hf_nodes -> freq;
    
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
			*zlib_err = -ZLIB_IO_ERROR;
			return *zlib_err;
		}

		parent[parent_size - 1] = 0;

        // Assign parents for tree traversal
        parent[left.symbol] = new_node.symbol;
        parent[right.symbol] = new_node.symbol;

		update_hf_nodes(new_node, hf_nodes, hf_nodes_cnt);
    }

    // Assign bit-lengths from depths
	for (unsigned short int i = 0; i < hf_tree -> size; i++) {
        unsigned int node = i;
        while (parent[node]) {
			((hf_tree -> lengths)[i])++;
            node = parent[node];
        }
    }

	XCOMP_SAFE_FREE(parent);
	return ZLIB_NO_ERROR;
}

// Compute Huffman code lengths from a frequency values
static int generate_hf_tree(const unsigned int* freqs, HFTree* hf_tree, int* zlib_err) {
	HFNode hf_nodes[MAX_HF_SIZE] = {0};  // Heap for building the tree
	unsigned short int hf_nodes_cnt = 0;
	for (unsigned short int i = 0; i < MAX_HF_SIZE; ++i) {
		if (freqs[i]) {
			HFNode new_node = (HFNode) { .symbol = i, .freq = freqs[i] };
			update_hf_nodes(new_node, hf_nodes, ++hf_nodes_cnt);
			hf_tree -> size = MAX(hf_tree -> size, i + 1);
		}
	}

	hf_tree -> values = (unsigned short int*) xcomp_calloc(MAX(hf_tree -> size, 1), sizeof(unsigned short int));
	hf_tree -> lengths = (unsigned char*) xcomp_calloc(MAX(hf_tree -> size, 1), sizeof(unsigned char));
	if (hf_tree -> values == NULL || hf_tree -> lengths == NULL) {
		deallocate_hf_tree(hf_tree);
		WARNING_LOG("Failed to allocate buffer for hf_tree.\n");
		*zlib_err = -ZLIB_IO_ERROR;
		return *zlib_err;
	}
	
	if (hf_tree -> size == 0) return ZLIB_NO_ERROR;

	if (compute_hf_codes(hf_nodes, hf_nodes_cnt, hf_tree, zlib_err) < 0) {
		WARNING_LOG("An error occurred while computing the hf codes.\n");
		return *zlib_err;
	}

	build_hf_tree(hf_tree);

	return ZLIB_NO_ERROR;

}

static int generate_hf_trees_from_matches(Matches* distance_encoded, HFTree* hf_literals, HFTree* hf_distances, int* zlib_err) {
	unsigned int lit_freqs[MAX_HF_SIZE]  = {0};
	unsigned int dist_freqs[MAX_HF_SIZE] = {0};
	for (unsigned int i = 0; i < distance_encoded -> cnt; ++i) { 
		const unsigned int lit  = (distance_encoded -> literal)[i];
		const unsigned int dist = (distance_encoded -> distance)[i]; 
		if (lit > 256) dist_freqs[dist]++;
		lit_freqs[lit]++;
	}

	if (generate_hf_tree(lit_freqs, hf_literals, zlib_err) < 0) {
		WARNING_LOG("An error occurred while generating the hf_tree for the previous hf.\n");
		return *zlib_err;
	}

	if (generate_hf_tree(dist_freqs, hf_distances, zlib_err) < 0) {
		WARNING_LOG("An error occurred while generating the hf_tree for the previous hf.\n");
		deallocate_hf_tree(hf_literals);
		return *zlib_err;
	}

	return ZLIB_NO_ERROR;
}

static int generate_hf_tree_from_rle(const RLEStream rle_encoded, HFTree* hf_tree, int* zlib_err) {
	unsigned int freqs[MAX_HF_SIZE] = {0};
	for (unsigned int i = 0; i < rle_encoded.cnt; ++i) { 
		const unsigned char value = (rle_encoded.values)[i];
		unsigned int repeat_cnt = (rle_encoded.repeat_cnts)[i];
		repeat_cnt--, freqs[value]++;
		
		while (repeat_cnt >= 3) {
			const unsigned char rep_value = 16 + (value == 0) + (value == 0 && repeat_cnt > 10);
			freqs[rep_value]++;
			if (value != 0) repeat_cnt -= MIN(repeat_cnt, 6);
		    else            repeat_cnt -= MIN(repeat_cnt, 138);
		}
		
		while (repeat_cnt--) freqs[value]++;
	}

	
	if (generate_hf_tree(freqs, hf_tree, zlib_err) < 0) {
		WARNING_LOG("An error occurred while generating the hf_tree for the previous hf.\n");
		return *zlib_err;
	}
	
	return ZLIB_NO_ERROR;
}

static int write_encoded_hf_trees(RLEStream rle_encoded, HFTree hf_tree, BitStream* bit_stream, int* zlib_err) {
	for (unsigned short int i = 0; i < rle_encoded.cnt; ++i) {
		const unsigned char value = (rle_encoded.values)[i];
		unsigned int repeat_cnt   = (rle_encoded.repeat_cnts)[i];
		bitstream_write_bits_reversed(bit_stream, (hf_tree.lengths)[value], hf_tree.values + value);
		repeat_cnt--;

		while (repeat_cnt >= 3) {
			const unsigned char rep_value = 16 + (value == 0) + (value == 0 && repeat_cnt > 10);
			bitstream_write_bits_reversed(bit_stream, (hf_tree.lengths)[rep_value], hf_tree.values + rep_value);
			
			unsigned char rep_cnt = 0;
			if (value != 0) rep_cnt = MIN(repeat_cnt, 6);
		    else            rep_cnt = MIN(repeat_cnt, 138);
			repeat_cnt -= rep_cnt;

			const unsigned char nbits = 2 + (rep_value == 17) + 5 * (rep_value == 18);
			rep_cnt -= 3 + 8 * (rep_value == 18);
			bitstream_write_bits(bit_stream, nbits, &rep_cnt);
		}

		while (repeat_cnt--) {
			bitstream_write_bits_reversed(bit_stream, (hf_tree.lengths)[value], hf_tree.values + value);
		}

		if (bit_stream -> error) {
			WARNING_LOG("An error occurred while generating the hf_tree for the previous hf.\n");
			*zlib_err = bit_stream -> error;
			return *zlib_err;
		}
	}
	
	return ZLIB_NO_ERROR;
}

static int generate_hf_trees(Matches* distance_encoded, BitStream* bit_stream, HFTree* hf_literals, HFTree* hf_distances, int* zlib_err) {
	if (generate_hf_trees_from_matches(distance_encoded, hf_literals, hf_distances, zlib_err) < 0) {
		WARNING_LOG("An error occurred while generating the hf_tree for the previous hf.\n");
		return *zlib_err;
	}

	RLEStream rle_encoded = {0};
	if (rle_encoding(&rle_encoded, *hf_literals, *hf_distances, zlib_err) < 0) {
		deallocate_hf_tree(hf_literals);
		deallocate_hf_tree(hf_distances);
		WARNING_LOG("An error occurred while rle_encoding.\n");
		return *zlib_err;
	}

	HFTree hf_tree = { .size = HF_TABLE_SIZE };
	if (generate_hf_tree_from_rle(rle_encoded, &hf_tree, zlib_err) < 0) {
		XCOMP_MULTI_FREE(rle_encoded.values, rle_encoded.repeat_cnts);
		deallocate_hf_tree(hf_literals);
		deallocate_hf_tree(hf_distances);
		WARNING_LOG("An error occurred while generating the hf_tree for the previous hf.\n");
		return *zlib_err;
	}

	const unsigned char hlit = MAX(257, hf_literals -> size) - 257;
	bitstream_write_bits(bit_stream, 5, &hlit);
	const unsigned char hdist = MAX(1, hf_distances -> size) - 1;
	bitstream_write_bits(bit_stream, 5, &hdist);
	
	const unsigned char order_of_code_lengths[] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
	unsigned char hclen = 18;
	for (; hclen > 4; --hclen) if ((hf_tree.lengths)[order_of_code_lengths[hclen]] > 0) break;
	hclen -= 3;
	bitstream_write_bits(bit_stream, 4, &hclen);

	for (unsigned char i = 0; i < hclen + 4; ++i) {
		bitstream_write_bits(bit_stream, 3, hf_tree.lengths + order_of_code_lengths[i]);
	}

	if (write_encoded_hf_trees(rle_encoded, hf_tree, bit_stream, zlib_err) < 0) {
		XCOMP_MULTI_FREE(rle_encoded.values, rle_encoded.repeat_cnts);
		deallocate_hf_tree(&hf_tree);
		deallocate_hf_tree(hf_literals);
		deallocate_hf_tree(hf_distances);
		WARNING_LOG("An error occurred while writing the RLE encoded hf trees.\n");
		return *zlib_err;
	}

	deallocate_hf_tree(&hf_tree);
	XCOMP_MULTI_FREE(rle_encoded.values, rle_encoded.repeat_cnts);

	return ZLIB_NO_ERROR;
}

static void deallocate_hf_tree(HFTree* hf_tree) {
	XCOMP_SAFE_FREE(hf_tree -> lengths);
	XCOMP_SAFE_FREE(hf_tree -> values);
	return;
}

static int hf_encode_block(HFTree hf_lit, HFTree hf_dist, Matches* distance_encoding, BitStream* bit_stream, int* zlib_err) {
    const unsigned char lenghts_extra_bits[29]   = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    const unsigned char distances_extra_bits[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

	if (hf_lit.lengths  == NULL) hf_lit.lengths  = (unsigned char*)      fhf_lit_lengths;
	if (hf_lit.values   == NULL) hf_lit.values   = (unsigned short int*) fhf_lit_values;
	if (hf_dist.lengths == NULL) hf_dist.lengths = (unsigned char*)      fhf_dist_lengths;
	if (hf_dist.values  == NULL) hf_dist.values  = (unsigned short int*) fhf_dist_values;

	for (unsigned int i = 0; i < distance_encoding -> cnt; ++i) {
		unsigned short int literal = (distance_encoding -> literal)[i];
		bitstream_write_bits_reversed(bit_stream, (hf_lit.lengths)[literal], hf_lit.values + literal);
		if (bit_stream -> error) break;
		else if (literal > 256) {
			unsigned char distance = (distance_encoding -> distance)[i];
			bitstream_write_bits(bit_stream, lenghts_extra_bits[literal - 257], &((distance_encoding -> length_diff)[i]));
			bitstream_write_bits_reversed(bit_stream, (hf_dist.lengths)[distance], hf_dist.values + distance);
			bitstream_write_bits(bit_stream, distances_extra_bits[distance], &((distance_encoding -> distance_diff)[i]));
			if (bit_stream -> error) break;
		}
	}
	
	if (hf_lit.lengths != fhf_lit_lengths) {
		deallocate_hf_tree(&hf_lit);
		deallocate_hf_tree(&hf_dist);
	}

	XCOMP_SAFE_FREE(distance_encoding -> literal);
	XCOMP_SAFE_FREE(distance_encoding -> distance);
	XCOMP_SAFE_FREE(distance_encoding -> length_diff);
	XCOMP_SAFE_FREE(distance_encoding -> distance_diff);

	if (bit_stream -> error) {
		*zlib_err = bit_stream -> error;
		return *zlib_err;
	}

	return ZLIB_NO_ERROR;
}

static int hf_compress_block(const zlib_block_t* block, BitStream* compressed_bitstream, zlib_buffer_t* buffer, const unsigned int window_size, int* zlib_err) {
	Matches distance_encoding = {0};
	if (length_distance_encoding(buffer, &distance_encoding, window_size, zlib_err) < 0) {
		WARNING_LOG("An error occurred while performing the length-distance encoding.\n");
		return *zlib_err; 
	}

	HFTree hf_lit = {0};
	HFTree hf_dist = {0};
	if (block -> compression_method == COMPRESSED_DYNAMIC_HF) {
		if (generate_hf_trees(&distance_encoding, compressed_bitstream, &hf_lit, &hf_dist, zlib_err) < 0) {
			WARNING_LOG("An error occurred while generating hf_tables.\n");
			return *zlib_err;
		}
	}
	
	// Huffman encode the block, encapsulating it into a DEFLATE block (append
	// block header, encoded data plus the encoded '256' to signal the end of
	// the block)
	if (hf_encode_block(hf_lit, hf_dist, &distance_encoding, compressed_bitstream, zlib_err) < 0) {
		WARNING_LOG("An error occurred while encoding the block.\n");
		return *zlib_err;
	} 
	
	return *zlib_err;
}

static int encode_uncompressed_block(BitStream* compressed_bitstream, zlib_buffer_t* buffer, const unsigned int window_size, int* zlib_err) {	
	const unsigned short int buffer_len = MIN(buffer -> size - buffer -> pos, window_size);
	bitstream_write_bytes(compressed_bitstream, sizeof(unsigned short int), 1, &buffer_len);
	if (compressed_bitstream -> error) { 
		*zlib_err = -(compressed_bitstream -> error);
		return *zlib_err;
	}
	
	const unsigned short int buffer_clen = ~buffer_len;
	bitstream_write_bytes(compressed_bitstream, sizeof(unsigned short int), 1, &buffer_clen);
	if (compressed_bitstream -> error) { 
		*zlib_err = -(compressed_bitstream -> error);
		return *zlib_err;
	}
	
	bitstream_write_bytes(compressed_bitstream, sizeof(unsigned char), buffer_len, buffer -> data + buffer -> pos);
	if (compressed_bitstream -> error) { 
		*zlib_err = -(compressed_bitstream -> error);
		return *zlib_err;
	}
	
	buffer -> pos += buffer_len;

	return ZLIB_NO_ERROR;
}

static int write_block_header(BitStream* bit_stream, const zlib_block_t* block, int* zlib_err) {
	bitstream_write_bits(bit_stream, 3, block); 
	if (bit_stream -> error) { 
		*zlib_err = -(bit_stream -> error);
		return *zlib_err;
	}
	return 0;
}

static int compress_block(BitStream* compressed_bitstream, zlib_buffer_t* buffer, const unsigned int window_size, int* zlib_err) {
	const BType compression_method =  COMPRESSED_DYNAMIC_HF; //COMPRESSED_FIXED_HF; // NO_COMPRESSION; 
	// const BType compression_method = choose_compression_method(buffer, window_size, zlib_err);
	const zlib_block_t block = { .is_final = (buffer -> size - buffer -> pos) <= window_size, .compression_method = compression_method };
	if (write_block_header(compressed_bitstream, &block, zlib_err) < 0) return *zlib_err;
	
	if (block.compression_method == NO_COMPRESSION) {
	    if (encode_uncompressed_block(compressed_bitstream, buffer, window_size, zlib_err) < 0) {
			WARNING_LOG("An error occurred while encoding the uncompressed block.\n");
		}
		return *zlib_err;
	}

	if (hf_compress_block(&block, compressed_bitstream, buffer, window_size, zlib_err) < 0) {
		WARNING_LOG("An error occurred while compressing the block with huffman coding.\n");
		return *zlib_err;
	}

	return ZLIB_NO_ERROR;
}

static int zlib_raw_deflate(BitStream* compressed_bitstream, zlib_buffer_t* buffer, const unsigned int window_size, int* zlib_err) {
	// Fragment the data in block of WINDOW_SIZE
	unsigned int block_cnt = 0;
	while (buffer -> size > buffer -> pos) {
		const unsigned char is_final = (buffer -> size - buffer -> pos) <= window_size;
		
		DEBUG_LOG("Block %u: is_final: %u", ++block_cnt, is_final);
		if (compress_block(compressed_bitstream, buffer, window_size, zlib_err) < 0) {
			XCOMP_SAFE_FREE(buffer -> data);
			deallocate_bit_stream(compressed_bitstream);
			WARNING_LOG("An error occurred while compressing the block.");
			return *zlib_err;
		}
	}
	
	XCOMP_SAFE_FREE(buffer -> data);
	
	return 0;
}

static int write_zlib_header(BitStream* compressed_bitstream, zlib_header_t* zlib_header) {
	if (zlib_header -> compression_method != 8) return -ZLIB_INVALID_COMPRESSION_METHOD;
	else if (zlib_header -> window_size > 7)    return -ZLIB_INVALID_WINDOW_SIZE;
	else if (zlib_header -> preset_dictionary)  return -ZLIB_DICTIONARY_NOT_SUPPORTED;
	
	DEBUG_LOG("-- ZLIB HEADER --");
	DEBUG_LOG("compression method: %u", zlib_header -> compression_method);
	DEBUG_LOG("window size:        %u", zlib_header -> window_size);
	DEBUG_LOG("preset dictionary:  %u", zlib_header -> preset_dictionary);
	DEBUG_LOG("compression level:  %u", zlib_header -> compression_level);
	DEBUG_LOG("-----------------");

	unsigned char compress_data = zlib_header -> compression_method & 0x0F;
    compress_data |= (zlib_header -> window_size & 0x0F) << 4;
    unsigned char flags = (zlib_header -> preset_dictionary & 0x01) << 5;
    flags |= (zlib_header -> compression_level & 0x03) << 6;

	if ((compress_data * 256 + flags) % 31 != 0) {
		flags += 31 - ((compress_data * 256 + flags) % 31);
		if ((compress_data * 256 + flags) % 31 != 0) return -ZLIB_INVALID_CHECKSUM;
	}

	bitstream_write_byte(compressed_bitstream, compress_data);
	if (compressed_bitstream -> error) {
		deallocate_bit_stream(compressed_bitstream);
		return -ZLIB_IO_ERROR;
	}

	bitstream_write_byte(compressed_bitstream, flags);
	if (compressed_bitstream -> error) {
		deallocate_bit_stream(compressed_bitstream);
		return -ZLIB_IO_ERROR;
	}

    return 0;
}

/* -------------------------------------------------------------------------------------------------------- */
unsigned char* deflate_deflate(unsigned char* data, unsigned int data_len, unsigned int* compressed_data_len, int* zlib_err) {
	BitStream compressed_bitstream = CREATE_BIT_STREAM(NULL, 0);
	zlib_buffer_t buffer = { .data = data, .size = data_len, .pos = 0 }; 
	if (zlib_raw_deflate(&compressed_bitstream, &buffer, WINDOW_SIZE, zlib_err) < 0) return NULL;
    *compressed_data_len = compressed_bitstream.size;
	return compressed_bitstream.stream;
}

unsigned char* zlib_deflate(unsigned char* data, const unsigned int data_len, unsigned int* compressed_data_len, zlib_header_t* zlib_header, int* zlib_err) {
	BitStream compressed_bitstream = CREATE_BIT_STREAM(NULL, 0);
	zlib_header_t default_zlib_header = { .compression_method = 8, .window_size = 7, .preset_dictionary = 0, .compression_level = 1 };
	if (zlib_header == NULL) zlib_header = &default_zlib_header;

	*zlib_err = write_zlib_header(&compressed_bitstream, zlib_header);
    if (*zlib_err < 0) {
		XCOMP_SAFE_FREE(data);
		return NULL;
	}

	// Calculate the ADLER-CRC of the blocks
	unsigned int adler_crc = __adler_crc(data, data_len, 1);
	XCOMP_BE_CONVERT(&adler_crc, sizeof(unsigned int));
	
	const unsigned int window_size = 1 << (zlib_header -> window_size + 8);
	zlib_buffer_t buffer = { .data = data, .size = data_len, .pos = 0 }; 
	if (zlib_raw_deflate(&compressed_bitstream, &buffer, window_size, zlib_err) < 0) return NULL;
	
	bitstream_write_bytes(&compressed_bitstream, sizeof(unsigned int), 1, &adler_crc);
	if (compressed_bitstream.error) { 
		*zlib_err = -(compressed_bitstream.error);
		deallocate_bit_stream(&compressed_bitstream);
		return NULL;
	}
	
    *compressed_data_len = compressed_bitstream.size;
	return compressed_bitstream.stream;
}

#endif
