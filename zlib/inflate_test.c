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
#include "./xcomp_zlib.h"

const unsigned char zlib_compressed_data[] = {
	0x0B, 0xC9, 0xC8, 0x2C, 0x56, 0x00, 0xA2, 0x44, 0x85, 0x92, 0xD4, 0xE2,
	0x12, 0x85, 0xE2, 0x92, 0xA2, 0xCC, 0xBC, 0x74, 0x85, 0x92, 0x7C, 0x85,
	0xE4, 0xFC, 0xDC, 0x82, 0xA2, 0xD4, 0x62, 0xA0, 0x4C, 0x5E, 0x8A, 0x42,
	0x4A, 0x2A, 0x9C, 0x5B, 0x5A, 0x0C, 0x52, 0xE0, 0xE2, 0xEA, 0xE6, 0xE3,
	0x18, 0xE2, 0xAA, 0x07, 0x00
};

int main(void) {
	int err = 0;
	
	unsigned int zlib_decompressed_data_length = 0;
	unsigned char* zlib_deflate_test = (unsigned char*) xcomp_calloc(sizeof(zlib_compressed_data), sizeof(unsigned char));
	
	mem_cpy(zlib_deflate_test, zlib_compressed_data, sizeof(zlib_compressed_data));
	
	unsigned char* zlib_decompressed_data = zlib_inflate((unsigned char*) zlib_deflate_test, sizeof(zlib_compressed_data), &zlib_decompressed_data_length, &err);
	if (err) {
		printf(COLOR_STR("ZLIB_ERROR::%s: ", RED) "%s", zlib_errors_str[-err], zlib_decompressed_data);
		return;
	}
	
	DEBUG_LOG("Zlib decompressed data: '%.*s'\n", zlib_decompressed_data_length, zlib_decompressed_data);

	xcomp_free(zlib_decompressed_data);

	return 0;
}

