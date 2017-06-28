/**
 * string_utils.c
 *
 * Some useful string functions.
 *
 * @author Connor Henley, @thatging3rkid
 */
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

/**
 * @inheritDoc
 */
char * str_trim(char * string) {
    int final_len = strlen(string);
    int i = 0;
    const int len = strlen(string);

    // Make sure the string is long enough
    if (len == 0) {
        char * output = malloc(sizeof(char));
        output[0] = '\0';
        return output;
    }
    
    if (len == 1) {
        char * output;
        if (string[0] != ' ') {
            output = malloc(sizeof(char) * 2);
            output[0] = string[0];
            output[1] = '\0';
        } else {
            output = malloc(sizeof(char));
            output[0] = '\0';
        }
        free(string);
        return output;
    }

    // Start from the front
    while (true) {
        if (string[i] != ' ') {
            break;
        } else {
            final_len -= 1;
            i += 1;
        }
    }

    // Then go to the back
    int k = len - 1;
    while (true) {
        if (string[k] != ' ') {
            break;
        } else {
            final_len -= 1;
            k -= 1;
        }
    }

    // Then copy the values out of the middle into the new array
    char * output = malloc(sizeof(char) * (final_len + 1));
    
    for (int l = 0; l < final_len; l += 1) {
        output[l] = string[l + i];
    }

    output[final_len] = '\0';
    free(string);
    return output;
}

/**
 * @inheritDoc
 */
char * str_parse(char * input) {
    int len = 0;

    if (strlen(input) > 2 && input[0] == '0' && input[1] == 'x') {
        len = 2;
    }
    
    while(isdigit(input[len]) || input[len] == 'a' || input[len] == 'b' || input[len] == 'c' ||
          input[len] == 'd' || input[len] == 'e' || input[len] == 'f') {
        len += 1;
    }
    len += 1; // add space for unl-terminator

    char * output = malloc(sizeof(char) * len);
    for (int i = 0; i < (len - 1); i += 1) {
        output[i] = input[i];
    }
    output[len - 1] = '\0';
    
    return output;
}

/**
 * @inheritDoc
 */
int str_guessbase(char * input) {
    if (strlen(input) > 2 && input[0] == '0' && input[1] == 'x') {
        return 16;
    } else {
        return 10;
    }
}
