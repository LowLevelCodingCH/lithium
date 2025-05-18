build:
	gcc -c -o lith.o lith.c -lm -lc
	ar rcs liblith.a lith.o
	cp liblith.a /usr/lib/liblith.a
	cp lith.h /usr/include/lith.h
	gcc -o main main.c -lm -llith
clean:
	rm lith.o
	rm liblith.a
	rm main
