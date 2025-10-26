FLAGS = -std=gnu11 -Wall -Wextra -pedantic -ggdb
DEFINITIONS = -D_DEBUG
EXTRA_DEFS = -D_XCOMP_PRINTING_UTILS_ -D_XCOMP_NO_PERROR_
XCOMP_HEADERS = zstd/xcomp_zstd.h zlib/xcomp_zlib.h 

static:     ./build/libxcomp.a
dynamic:    ./build/libxcomp.so
amalgamate: ./build/xcomp.h

# TODO: The current amalgamation process is not really going well, maybe should
# find a better approach, to prevent empty lines to be removed
./build/xcomp.h: $(XCOMP_HEADERS)
	echo "/*" > $@
	echo " * Copyright (C) 2025 TheProgxy <theprogxy@gmail.com>" >> $@
	echo " *" >> $@
	echo " * This program is free software: you can redistribute it and/or modify" >> $@
	echo " * it under the terms of the GNU General Public License as published by" >> $@
	echo " * the Free Software Foundation, either version 3 of the License, or" >> $@
	echo " * (at your option) any later version." >> $@
	echo " *" >> $@
	echo " * This program is distributed in the hope that it will be useful," >> $@
	echo " * but WITHOUT ANY WARRANTY; without even the implied warranty of" >> $@
	echo " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the" >> $@
	echo " * GNU General Public License for more details." >> $@
	echo " *" >> $@
	echo " * You should have received a copy of the GNU General Public License" >> $@
	echo " * along with this program. If not, see <https://www.gnu.org/licenses/>." >> $@
	echo " */" >> $@
	echo "#ifndef _XCOMP_H_" >> $@
	echo "#define _XCOMP_H_" >> $@
	echo "#include <stdio.h>" >> $@
	echo "#include <stdlib.h>" >> $@
	cat common/utils.h >> $@
	cat common/bitstream.h >> $@
	gcc $(DEFINITIONS) -D_XCOMP_UTILS_IMPLEMENTATION_ -E -C -P $(XCOMP_HEADERS) >> $@
	echo >> $@
	echo "#endif /* _XCOMP_H_ */" >> $@

test:
	$(MAKE) -C ./zstd test
	$(MAKE) -C ./zlib test

./build/libxcomp.so: build/xcomp.h build/xcomp_lib.h zstd/xcomp_zstd.h zlib/xcomp_zlib.h
	gcc $(FLAGS) $(DEFINITIONS) $(EXTRA_DEFS) -c -fPIC ./build/xcomp.h -shared -o $@

./build/libxcomp.a: build/xcomp.h build/xcomp_lib.h zstd/xcomp_zstd.h zlib/xcomp_zlib.h
	gcc $(FLAGS) $(DEFINITIONS) $(EXTRA_DEFS) -c -fPIC ./build/xcomp.h -o xcomp.o
	ar rcs $@ xcomp.o
	rm -rf xcomp.o

# TODO: Add support for Windows DLLs (as well as compiling with msvc)
# xcomp_lib.dll: xcomp_lib.h
# 	gcc $(FLAGS) $(DEFINITIONS) $< -o $@


