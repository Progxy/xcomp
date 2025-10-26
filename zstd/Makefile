FLAGS = -std=gnu11 -Wall -Wextra -pedantic -ggdb
DEFINITIONS = -D_DEBUG

test: zstd_tester
	./zstd_automated_test.sh

inflate_test: inflate_test.c xcomp_zstd.h
	gcc $(FLAGS) $(DEFINITIONS) $< -o $@

deflate_test: deflate_test.c xcomp_zstd.h
	gcc $(FLAGS) $(DEFINITIONS) $< -o $@

zstd_tester: zstd_tester.c xcomp_zstd.h
	gcc $(FLAGS) $< -o $@

