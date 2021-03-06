# Makefile for the Delta text editor
#
# @author Connor Henley, @thatging3rkid

# Define gcc as the compiler, because I like gcc
# Theoretically, this can be changed, but it's all on you then
cc = gcc

# Define flags for a debug build (which should be done before an install)
debug_flags = -Wall -Wextra -pedantic -std=c99 -g -ggdb

# Define flags for a final build (try and make it go super fast)
build_flags = -std=c99 -finline-functions -o2

# Define required library flags
libflags = -lncurses -lm

# debug is the default make, runs a debug make
debug:
	$(cc) $(debug_flags) delta.c -o delta $(libflags)

# build makes a production level build
build:
	@echo Building production build of Delta...
	rm delta
	$(cc) $(build_flags) delta.c -o delta $(libflags)

# clean removes all the object files and executable
clean:
	rm delta

# install installs the program on the system
install:
	@echo Installation not yet recommended
	@echo Run \'sudo make yes-i-really-want-to-install-this-editor-now\' to install

# install-dependencies installs ncurses so that Delta can be compiled
install-dependencies:
	@echo Installing dependencies for delta using apt-get...
	sudo apt-get install libncurses-dev

# Actually installs the program
yes-i-really-want-to-install-this-editor-now:
	@echo Installing Delta...
	sudo $(cc) $(build_flags) delta.c -o /usr/bin/delta $(libflags)	
	sudo chown root:root /usr/bin/delta
	sudo chmod 755 /usr/bin/delta
	@echo Delta installed
