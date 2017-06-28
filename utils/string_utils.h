/**
 * string_utils.h
 *
 * Some useful string functions.
 *
 * @author Connor Henley, @thatging3rkid
 */
#ifndef STRING_UTILS_LIB
#define STRING_UTILS_LIB

/**
 * A structure for a String
 */
typedef struct {
    char * data;
    int len;
} String;

/**
 * Trim the spaces from a string
 *
 * @param string the raw string
 * @return a string without any spaces beginning or trailing the string
 *
 * @note nul-terminated strings are used
 * @note the returned string must be freed
 */
char * str_trim(char * str);

/**
 * Find the next numbers in a string
 *
 * @param input the string to look at
 * @return the parsed string
 *
 * @note the returned string must be freed
 */
char * str_parse(char * input);

/**
 * Try and guess the base of a number (10 or 16)
 *
 * @param input the string containing the number to guess with
 * @return 10 or 16, depending on the contents of the string
 */
int str_guessbase(char * input);

#endif
