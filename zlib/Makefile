FLAGS = -std=gnu11 -Wall -Wextra -pedantic -ggdb
DEFINITIONS = -D_DEBUG

test: zlib_tester
	./zlib_automated_test.sh

inflate_test: inflate_test.c xcomp_zlib.h 
	gcc $(FLAGS) $(DEFINITIONS) $< -o $@

deflate_test: deflate_test.c xcomp_zlib.h 
	gcc $(FLAGS) $(DEFINITIONS) $< -o $@

zlib_tester: zlib_tester.c xcomp_zlib.h 	
	gcc $(FLAGS) $< -o $@

