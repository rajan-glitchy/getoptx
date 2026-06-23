CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -pedantic -O2
CPPFLAGS ?= -I.

.PHONY: all check clean format

all: examples/basic

examples/basic: examples/basic.c getoptx.c getoptx.h
	$(CC) $(CPPFLAGS) $(CFLAGS) getoptx.c examples/basic.c -o $@

tests/test_getoptx: tests/test_getoptx.c getoptx.c getoptx.h
	$(CC) $(CPPFLAGS) $(CFLAGS) getoptx.c tests/test_getoptx.c -o $@

check: tests/test_getoptx
	./tests/test_getoptx

clean:
	rm -f examples/basic tests/test_getoptx

format:
	clang-format -i getoptx.c getoptx.h examples/basic.c tests/test_getoptx.c