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

#define _QCOW_PRINTING_UTILS_
#define _QCOW_UTILS_IMPLEMENTATION_
#include "utils.h"
#include "./zstd.h"

int main(int argc, char* argv[]) {
	if (argc > 3) {
		printf("Usage: zstd_tester <file> [<out_file>]\n");
		return -1;
	}
	
	const char* file_path = argv[1];
	const char* out_file_path = (argc > 2) ? argv[2] : NULL;
	DEBUG_LOG("Decoding '%s' to '%s'...\n", file_path, out_file_path == NULL ? "stdin" : out_file_path);

	FILE* test_file = NULL;
	if ((test_file = fopen(file_path, "rb")) == NULL) {
		PERROR_LOG("Failed to open the test file");
		return -1;
	}

	fseek(test_file, 0, SEEK_END);
	long file_size = ftell(test_file);
	fseek(test_file, 0, SEEK_SET);

	DEBUG_LOG("File size: %lu\n", file_size);
	
	size_t file_err = 0;
	unsigned char* test_data = (unsigned char*) qcow_calloc(file_size, sizeof(unsigned char));
	if (((long)(file_err = fread(test_data, sizeof(unsigned char), file_size, test_file)) != file_size)) {
		PERROR_LOG("Failed to read from the test file");
		qcow_free(test_data);
		fclose(test_file);
		return -1;
	} 	
	
	fclose(test_file);

	int err = 0;
	unsigned int zstd_decompressed_data_length = 0;
	unsigned char* zstd_decompressed_data = zstd_inflate((unsigned char*) test_data, file_size, &zstd_decompressed_data_length, &err);
	if (err) {
		printf(COLOR_STR("ZSTD_ERROR::%s: ", RED) "%s", zstd_errors_str[-err], zstd_decompressed_data);
		return err;
	} 
	
	if (out_file_path == NULL) printf("ZSTD decompressed data: '%.*s'\n", zstd_decompressed_data_length, zstd_decompressed_data);
	else {
		FILE* out_file = NULL;
		if ((out_file = fopen(out_file_path, "wb")) == NULL) {
			PERROR_LOG("Failed to open the out file");
			return -1;
		}
		
		file_err = 0;
		if (((long)(file_err = fwrite(zstd_decompressed_data, sizeof(unsigned char), zstd_decompressed_data_length, out_file)) != zstd_decompressed_data_length)) {
			PERROR_LOG("Failed to write from the out file");
			qcow_free(zstd_decompressed_data);
			fclose(out_file);
			return -1;
		}

		fclose(out_file);
	}
	
	qcow_free(zstd_decompressed_data);
	
	return 0;
}

