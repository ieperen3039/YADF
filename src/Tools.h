//
// Created by ieperen3039 on 27-11-19.
//
#ifndef YADF_TOOLS_C
#define YADF_TOOLS_C

#include <stdio.h>
#include <errno.h>
#include "global.h"

/// reads the contents of the given file into a char[]. If strlen is not NULL, the length of the returned char[] is stored there.
static char* readFile(char* filename, int* strlen) {
    FILE* fp;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        LOG_ERROR_F("Could not open file %s", strerror(errno));
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

    if (strlen) *strlen = lSize;

    fclose(fp);
    return buffer;
}

#ifndef __linux__

/*
* public domain strtok_r() by Charlie Gordon
*
*   from comp.lang.c  9/14/2007
*
*      http://groups.google.com/group/comp.lang.c/msg/2ab1ecbb86646684
*
*     (Declaration that it's public domain):
*      http://groups.google.com/group/comp.lang.c/msg/7c7b39328fefab9c
*/

static char* strtok_r(char* str, const char* delim, char** nextp) {
    char* ret;

    if (str == NULL) {
        str = *nextp;
    }

    str += strspn(str, delim);

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

#endif

#endif // YADF_TOOLS_C