//
// Created by ieperen3039 on 27-11-19.
//
#ifndef YADF_TOOLS_C
#define YADF_TOOLS_C

#include "global.h"
#include "DataStructures/Map.h"

#include <stdio.h>
#include <errno.h>
#include <assert.h>

/// reads the contents of the given file into a char[]. If strlen is not NULL, the length of the returned char[] is stored there.
static char* tool_read_file(const char* filename, int* strlen){
    FILE* fp;

    fp = fopen(filename, "r");
    if (!fp) {
        LOG_ERROR_F("Could not open file '%s': %s", filename, strerror(errno));
        return NULL;
    }

    // obtain file size:
    fseek(fp, 0, SEEK_END);
    size_t lSize = ftell(fp);
    rewind(fp);

    // allocate memory to contain the whole file:
    char* buffer = (char*) malloc(sizeof(char) * lSize);
    if (buffer == NULL) {
        LOG_ERROR_F("Memory error: %s", strerror(errno));
        return NULL;
    }

    // copy the file into the buffer:
    size_t items_read = fread(buffer, 1, lSize, fp);
    if (ferror(fp)) {
        LOG_ERROR_F("IO error: %s", strerror(errno));
        return NULL;
    }
    buffer[items_read] = '\0'; // seems to miss sometimes

    if (strlen) *strlen = lSize;

    fclose(fp);
    return buffer;
}

/// replaces any line comment, block comment and newline with the given char
static void tool_strip_char(char* buffer, int strlen, char replacement){
    int i = 0;
    while (i < strlen) {
        if (buffer[i] == '/') {
            if (buffer[i + 1] == '/') {
                // line comment
                buffer[i++] = replacement;
                buffer[i++] = replacement;

                while (buffer[i] != '\n') {
                    buffer[i++] = replacement;
                }
                buffer[i++] = replacement;

            } else if (buffer[i + 1] == '*') {
                // block comment
                buffer[i++] = replacement;
                buffer[i++] = replacement;

                while (buffer[i] != '*' || buffer[i + 1] != '/') {
                    buffer[i++] = replacement;
                }

                buffer[i++] = replacement;
                buffer[i++] = replacement;

            } else {
                i++;
            }

        } else if (buffer[i] == '\n') {
            buffer[i++] = replacement;

        } else {
            i++;
        }
    }
};

PURE static inline int min_i(int a, int b) {
    return a > b ? b : a;
}

PURE static inline int max_i(int a, int b) {
    return a > b ? a : b;
}

#include <ctype.h>

/// Case insensitive string comparison, doesn't consider two NULL pointers equal
PURE static inline int tool_case_insensitive_strcmp(const char* string1, const char* string2){
    if ((string1 == NULL) || (string2 == NULL))
    {
        return 1;
    }

    if (string1 == string2)
    {
        return 0;
    }

    for(; tolower(*string1) == tolower(*string2); (void)string1++, string2++)
    {
        if (*string1 == '\0')
        {
            return 0;
        }
    }

    return tolower(*string1) - tolower(*string2);
};


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