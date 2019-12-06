//
// Created by ieperen3039 on 27-11-19.
//
#ifndef YADF_TOOLS_C
#define YADF_TOOLS_C
#include <stdio.h>
#include <errno.h>

/// reads the contents of the given file into a char[]. If strlen is not NULL, the length of the returned char[] is stored there.
static char* readFile(char* filename, int* strlen) {
    FILE* fp;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr,"Could not open file %s", strerror(errno));
        return NULL;
    }

    // obtain file size:
    fseek(fp, 0, SEEK_END);
    size_t lSize = ftell(fp);
    rewind(fp);

    // allocate memory to contain the whole file:
    char* buffer = (char*) malloc(sizeof(char) * lSize);
    if (buffer == NULL) {
        fprintf(stderr, "Memory error: %s", strerror(errno));
        return NULL;
    }

    // copy the file into the buffer:
    size_t items_read = fread(buffer, 1, lSize, fp);
    if (items_read != lSize) {
        fprintf(stderr, "IO error: %s", strerror(errno));
        return NULL;
    }

    if (strlen) *strlen = lSize;

    fclose(fp);
    return buffer;
}

#endif // YADF_TOOLS_C