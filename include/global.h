//
// Created by ieperen3039 on 25-11-19.
//

#ifndef YADF2_GLOBAL_H
#define YADF2_GLOBAL_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

typedef int ErrorCode;
#define ERROR_NONE 0
#define ERROR_OUT_OF_BOUNDS 1
#define ERROR_IO 2

#define LOG_INFO(msg) lprintf(stdout, __FILE__, __LINE__, msg, true)
#define LOG_INFO_F(fmt, msg) lprintf(stdout, __FILE__, __LINE__, "", false); fprintf(stdout, fmt, msg)
#define LOG_ERROR(msg) lprintf(stderr, __FILE__, __LINE__, msg, true)
#define LOG_ERROR_F(fmt, msg) lprintf(stderr, __FILE__, __LINE__, "", false); fprintf(stderr, fmt, msg)

typedef struct _Allocator {
    void* object;
    void* (*get)(struct _Allocator* this, size_t);
} Allocator;

/// malloc(size_t) as an Allocator
void* _system_malloc(Allocator* this, size_t size){
    return malloc(size);
}

#define SYSTEM_ALLOCATOR (Allcator){NULL, _system_malloc}

static void lprintf(FILE* out, const char* file_name, int line, const char* msg, bool newline) {
    char prefix[70];
    char* project_local = strstr(file_name, "YADF");
    snprintf(prefix, 70, "%s (line: %d)", project_local, line);
    fprintf(out, "%-70s : %s", prefix, msg);
    if (newline) fprintf(out, "\n");
}

#ifdef __GNUC__
/// PURE means that it doest change anything in the parameters
#define PURE __attribute__ ((pure))
#else
#define PURE
#endif // __GNUC__

#endif //YADF2_GLOBAL_H
