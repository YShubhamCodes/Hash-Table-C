CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -std=c11 -Iinclude
SRC     = src/hash_table.c

.PHONY: all test clean

# Default: build the demo binary
all: main

main: src/main.c $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Build and run tests
test: tests/test_hash_table.c $(SRC)
	$(CC) $(CFLAGS) -o test_runner $^
	./test_runner

clean:
	rm -f main test_runner
