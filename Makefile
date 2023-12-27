CFLAGS=-Wall -ggdb -std=c11 -pedantic

all: hexstrings_to_binary.c
	$(CC) $(CFLAGS) hexstrings_to_binary.c -o hexstrings_to_binary

clean: rm -f hexstrings_to_binary
