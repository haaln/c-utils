#ifndef UTIL_FILE_IO_H
#define UTIL_FILE_IO_H

struct string_t;

/**
 * Opens a file and reads in the contents into a string_t. Note that you need to free the string after using it.
 */
string_t *fopen_into_memory(const char *file);

#endif
