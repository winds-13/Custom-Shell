CC=gcc
CFLAGS=-g -pedantic -std=gnu17 -Wall -Werror -Wextra

.PHONY: all
all: nyush

nyush: nyush.o

nyuc.o: nyush.c

.PHONY: clean
clean:
	rm -f *.o nyuc
