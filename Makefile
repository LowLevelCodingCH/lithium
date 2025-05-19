# Uses clang because of better error logs
CC=clang

build:
	$(CC) -c -o lith.o lith.c
	ar rcs liblith.a lith.o
	cp liblith.a /usr/lib/liblith.a
	cp lith.h /usr/include/lith.h
	$(CC) -o main main.c -lm -llith
clean:
	rm lith.o
	rm liblith.a
	rm main
