//
// Created by ieperen3039 on 25-11-19.
//

#ifndef YADF2_GLOBAL_H
#define YADF2_GLOBAL_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
    ERROR_NONE              = 0,
    ERROR_OUT_OF_BOUNDS     = 0x0001,
    ERROR_IO                = 0x0002
} ErrorCode;

#define LOG_INFO(msg) lprintf(stdout, __FILE__, __LINE__, msg, true)
#define LOG_INFO_F(fmt, msg) lprintf(stdout, __FILE__, __LINE__, "", false); fprintf(stdout, fmt, msg); fprintf(stdout, "\n")

#define LOG_ERROR(msg) lprintf(stderr, __FILE__, __LINE__, msg, true)
#define LOG_ERROR_F(fmt, msg) lprintf(stderr, __FILE__, __LINE__, "", false); fprintf(stderr, fmt, msg); fprintf(stderr, "\n")

static void lprintf(FILE* out, const char* file_name, int line, const char* msg, bool newline) {
    char prefix[50];
    char* project_local = strstr(file_name, "YADF");
    snprintf(prefix, 50, "%s:%d: ", project_local, line);
    fprintf(out, "%-50s : %s", prefix, msg);
    if (newline) fprintf(out, "\n");
}

#ifdef __GNUC__
/// PURE means that it doesnt change anything in the parameters
#define PURE __attribute__ ((pure))
#else
#define PURE
#endif // __GNUC__

#endif //YADF2_GLOBAL_H
