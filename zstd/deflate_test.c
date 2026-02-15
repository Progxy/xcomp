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

#include <stdio.h>
#include <stdlib.h>

#define _XCOMP_PRINTING_UTILS_
#define _XCOMP_UTILS_IMPLEMENTATION_
#include "../common/utils.h"
#define _XCOMP_BITSTREAM_
#include "./xcomp_zstd.h"

static const char zstd_data[] = "This is a test string, DEFLATE.";

int main(void) {
	int err = 0;
	
	printf("ZSTD test string: '%s'\n", zstd_data);

	unsigned int zstd_compressed_data_length = 0;
	unsigned char* zstd_deflate_test = (unsigned char*) xcomp_calloc(sizeof(zstd_data), sizeof(unsigned char));
	mem_cpy(zstd_deflate_test, zstd_data, sizeof(zstd_data));
	
	unsigned char* zstd_compressed_data = zstd_deflate((unsigned char*) zstd_deflate_test, sizeof(zstd_data), &zstd_compressed_data_length, &err);
	if (err) {
		printf(COLOR_STR("ZSTD_ERROR::%s: ", RED) "failed to compress the string\n", zstd_errors_str[-err]);
		return -1;
	}
	
	printf("ZSTD compressed from %lu -> %u bytes.\n", sizeof(zstd_data), zstd_compressed_data_length);

	unsigned int zstd_decompressed_data_length = 0;
	unsigned char* zstd_decompressed_data = zstd_inflate(zstd_compressed_data, zstd_compressed_data_length, &zstd_decompressed_data_length, &err);
	if (err) {
		printf(COLOR_STR("ZSTD_ERROR::%s: ", RED) "%s\n", zstd_errors_str[-err], zstd_decompressed_data);
		return -1;
	}

	if (str_n_cmp(zstd_data, (const char*) zstd_decompressed_data, MIN(sizeof(zstd_data), zstd_decompressed_data_length)) != 0) {
		printf(COLOR_STR("ERROR: ", RED) "Failed to decompress/compress the string.\n");
		printf("Original String: '%s'\n", zstd_data);
		printf("String After   : '%.*s'\n", zstd_decompressed_data_length, zstd_decompressed_data);
		xcomp_free(zstd_decompressed_data);
		return -1;
	}
	
	printf("ZSTD test string: '%.*s'\n", zstd_decompressed_data_length, zstd_decompressed_data);
	xcomp_free(zstd_decompressed_data);
	
	return 0;
}
