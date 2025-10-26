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

unsigned char zstd_compressed_data[] = {
	0x28, 0xB5, 0x2F, 0xFD, 0x20, 0x44, 0xE5, 0x01, 0x00, 0x42, 0x04, 0x0E,
	0x14, 0xA0, 0xB5, 0x39, 0xF1, 0xB4, 0x24, 0x74, 0xC5, 0xAE, 0xA2, 0x6E,
	0x94, 0x8D, 0xA0, 0xFF, 0x9F, 0xDF, 0xFE, 0x67, 0x0D, 0x81, 0x6B, 0x4B,
	0x77, 0x24, 0x12, 0x86, 0xB9, 0x7B, 0x9E, 0x15, 0x1E, 0xD0, 0xB3, 0x18,
	0x51, 0xF5, 0x6E, 0x92, 0xDA, 0xBD, 0x84, 0x6C, 0x20, 0xB9, 0x03, 0x3C,
	0xA7, 0x90, 0x59, 0xB4, 0xA1, 0x4D, 0x21, 0x04, 0x00
};

int main(void) {
	int err = 0;
	
	unsigned int zstd_decompressed_data_length = 0;
	unsigned char* zstd_deflate_test = (unsigned char*) xcomp_calloc(sizeof(zstd_compressed_data), sizeof(unsigned char));
	
	mem_cpy(zstd_deflate_test, zstd_compressed_data, sizeof(zstd_compressed_data));
	
	unsigned char* zstd_decompressed_data = zstd_inflate((unsigned char*) zstd_deflate_test, sizeof(zstd_compressed_data), &zstd_decompressed_data_length, &err);
	if (err) {
	   printf(COLOR_STR("ZSTD_ERROR::%s: ", RED) "%s", zstd_errors_str[-err], zstd_decompressed_data);
	   return 1;
	}

	DEBUG_LOG("ZSTD decompressed data: '%.*s'\n", zstd_decompressed_data_length, zstd_decompressed_data);
	
	xcomp_free(zstd_decompressed_data);

	return 0;
}

