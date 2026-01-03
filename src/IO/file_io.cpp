#include "file_io.h"

#include "../types.h"
#include "malloc.h"
#include "stdio.h"

#include "file_io.h"
#include <stdlib.h>
#include <string.h>

string_t *fopen_into_memory(const char *file)
{
    string_t *string = (string_t *)malloc(sizeof(string_t));

    FILE *FileHandle = fopen(file, "r");
    if(FileHandle) {
        fseek(FileHandle, 0, SEEK_END);
        string->length = ftell(FileHandle);
        fseek(FileHandle, 0, SEEK_SET);

        string->data = (char *)malloc(string->length);
        fread(string->data, string->length, 1, FileHandle);

        fclose(FileHandle);
    }

    return string;
}

// FIXME:
string_t get_home_dir()
{
    string_t Result;

    char *p = getenv("HOME");

    if(p) Result.data = p;
    Result.length = strlen(Result.data);

    return Result;
}

// FIXME:
string_t get_config_dir()
{
    string_t Result;
    char *data = getenv("XDG_CONFIG_HOME");
    if(data) {
        Result.data = data;
        Result.length = strlen(data);
        return Result;
    }

    string_t tmp = get_home_dir();
    if(!tmp.data) {
        Result.data[tmp.length + 9 + 1];
        Result.length = tmp.length + 10;
        snprintf(tmp.data, Result.length, "%s/.config/", Result.data);
    }

    return Result;
}
