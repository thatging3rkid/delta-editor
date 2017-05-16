# Makefile for the Delta text editor
#
# @author Connor Henley, @thatging3rkid

# Define gcc as the compiler, because I like gcc
# Theoretically, this can be changed, but it's all on you then
cc = gcc

# Define flags for a debug build (which should be done before an install)
debug_flags = -Wall -Wextra -pedantic -std=c99 -ggdb

# Define flags for a final build
build_flags = -std=c99

# Define required library flags
libflags = -lncurses -lm

# all is the default make, runs a debug make
all:
	$(cc) $(debug_flags) delta.c -o delta $(libflags)

#build makes a production level build
build:
	@echo Delta\nBuilding production build...
	rm delta.o delta
	$(cc) $(build_flags) delta.c -o delta $(libflags)

# clean removes all the object files and executable
clean:
	rm delta.o delta

# install installs the program on the system
install:
	@echo Installation not yet recomended
	@echo Run `sudo make yes-i-really-want-to-install-this-editor-now` to install

# Actually "installs" the program
yes-i-really-want-to-install-this-editor-now:
	@echo Installing Delta...
	@echo Make sure you have run `make build`
	sudo cp delta /usr/bin
	sudo chown root:root /usr/bin/delta
	sudo chmod 755 /usr/bin/delta
	@echo Delta installed
