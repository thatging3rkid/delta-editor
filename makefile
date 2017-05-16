cc = gcc
flags = -Wall -Wextra -pedantic -std=c99 -ggdb
libflags = -lncurses -lm

all:
	$(cc) $(flags) delta.c -o delta $(libflags)

clean:
	rm delta.o delta.out

install:
	@echo Installation not yet supported
