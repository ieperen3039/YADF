//
// Created by ieperen3039 on 27-11-19.
//
#ifndef YADF_TOOLS_C
#define YADF_TOOLS_C

#include "global.h"
#include "DataStructures/Map.h"

#include <stdio.h>
#include <errno.h>
#include <io.h>
#include <assert.h>

/// reads the contents of the given file into a char[]. If strlen is not NULL, the length of the returned char[] is stored there.
char* tool_read_file(const char* filename, int* strlen);

typedef struct cJSON cJSON;

/// reads the JSON mapping of colors to names into the given map
/// these names are only valid for the lifetime of the cJSON map
ErrorCode tool_read_color_map(cJSON* mapping, ElementMap* map);

/// replaces any line comment, block comment and newline with the given char
void tool_strip_char(char* buffer, int strlen, char replacement);

PURE static inline int min_i(int a, int b) {
    return a > b ? b : a;
}

PURE static inline int max_i(int a, int b) {
    return a > b ? a : b;
}

/**
 * splits the given string by the delimiter.
 * @param str the string to split. If NULL, the state stored in nextp is used
 * @param delim the delimiter
 * @param nextp the state is written here
 * @param multiple if true, this will consider a group of delimiters as one
 * @return
 */
static char* tool_split(char* str, const char* delim, char** nextp, bool multiple) {
    char* ret;

    if (str == NULL) {
        str = *nextp;
    }

    if (multiple) {
        str += strspn(str, delim);
    }

    if (*str == '\0') {
        return NULL;
    }

    ret = str;
    str += strcspn(str, delim);

    if (*str) {
        *str++ = '\0';
    }

    *nextp = str;

    return ret;
}

#endif // YADF_TOOLS_C