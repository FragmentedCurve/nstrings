CFLAGS= -std=c99 -pedantic -pedantic-errors -Wall -I../
LIBSRC= ../nstrings.h

test: test.c $(LIBSRC)
	$(CC) $(CFLAGS) -o test test.c
	./test

clean:
	rm -f test

.PHONY: clean
