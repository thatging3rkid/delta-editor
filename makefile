cc = gcc
flags = -Wall -Wextra -pedantic -std=c99 -ggdb

all: delta.o
	$(cc) $(flags) delta.o -o delta.out
	rm delta.o

delta.o:
	$(cc) $(flags) -c delta.c

clean:
	rm delta.o delta.out
