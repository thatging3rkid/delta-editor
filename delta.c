/**
 * delta.c
 *
 * A simple Unix text editor
 *
 * @author Connor Henley, @thatging3rkid
 */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char * argv[]) {
  if (argc == 1) {
    fprintf(stderr, "Usage: delta file\n");
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS; // edit the file
  }
  
}
