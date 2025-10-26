FLAGS = -std=gnu11 -Wall -Wextra -pedantic -ggdb
DEFINITIONS = -D_DEBUG
XCOMP_HEADERS = zstd/xcomp_zstd.h zlib/xcomp_zlib.h

static:     ./build/libxcomp.a
dynamic:    ./build/libxcomp.so
amalgamate: ./build/xcomp.h

# TODO: The current amalgamation process is not really going well, maybe should
# find a better approach, to prevent empty lines to be removed
./build/xcomp.h: $(XCOMP_HEADERS)
	cat license_preamble.txt > $@
	echo "#ifndef _XCOMP_H_" >> $@
	echo "#define _XCOMP_H_" >> $@
	for hdr in $(XCOMP_HEADERS); do              \
		gcc $(DEFINITIONS) -E -C -P $$hdr >> $@; \
		echo >> $@;                              \
	done
	echo "#endif /* _XCOMP_H_ */" >> $@

test:
	$(MAKE) -C ./zstd test
	$(MAKE) -C ./zlib test

./build/libxcomp.so: build/xcomp.h build/xcomp_lib.h zstd/xcomp_zstd.h zlib/xcomp_zlib.h
	cd build
	gcc $(FLAGS) $(DEFINITIONS) -D_XCOMP_H_ -c -fPIC xcomp.h -shared -o $@
	rm -rf xcomp.c
	cd ..

./build/libxcomp.a: build/xcomp.h build/xcomp_lib.h zstd/xcomp_zstd.h zlib/xcomp_zlib.h
	cd build
	gcc $(FLAGS) $(DEFINITIONS) -D_XCOMP_H_ -c -fPIC xcomp.h -o xcomp.o
	ar rcs $@ xcomp.o
	rm -rf xcomp.c xcomp.o
	cd ..

# TODO: Add support for Windows DLLs (as well as compiling with msvc)
# xcomp_lib.dll: xcomp_lib.h
# 	gcc $(FLAGS) $(DEFINITIONS) $< -o $@


