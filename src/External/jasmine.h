//
// Created by ieperen3039 on 29-11-19.
//

#ifndef YADF2_JASMINE_H
#define YADF2_JASMINE_H

#include <stddef.h>
#include <stdlib.h>

/**
 * JSON type identifier. Basic types are:
 * 	o Object
 * 	o Array
 * 	o String
 * 	o Other primitive: number, boolean (true/false) or null
 */
typedef enum {
    JSMN_UNDEFINED = 0,
    JSMN_OBJECT = 1,
    JSMN_ARRAY = 2,
    JSMN_STRING = 3,
    JSMN_PRIMITIVE = 4
} jsmntype_t;

enum jsmnerr {
    /* Not enough tokens were provided */
            JSMN_ERROR_NOMEM = -1,
    /* Invalid character inside JSON string */
            JSMN_ERROR_INVAL = -2,
    /* The string is not a full JSON packet, more bytes expected */
            JSMN_ERROR_PART = -3
};


/**
 * JSON token description.
 * type		type (object, array, string etc.)
 * start	start position in JSON data string
 * end		end position in JSON data string
 */
typedef struct {
    jsmntype_t type;
    int start;
    int end;
    int size;
    int parent;
} jsmntok_t;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string.
 */
typedef struct {
    unsigned int pos;     /* offset in the JSON string */
    unsigned int toknext; /* next token to allocate */
    int toksuper;         /* superior token node, e.g. parent object or array */
} jsmn_parser;

/**
 * Create JSON parser over an array of tokens
 */
void jsmn_init(jsmn_parser* parser);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each
 * describing
 * a single JSON object.
 * 
 * Usage:
 * jsmntok_t t[128]; // We expect no more than 128 JSON tokens
 *
 * jsmn_parser p;
 * jsmn_init(&p);
 *
 * r = jsmn_parse(&p, s, strlen(s), t, 128);
 */
int jsmn_parse(
        jsmn_parser* parser, const char* js, size_t len, jsmntok_t* tokens, unsigned int max_tokens
);

/// parses the given token as long
static inline long jsmn_get_long(const char* js, jsmntok_t* token) {
    const char* offset = &(js[token->start]);
    return strtol(offset, NULL, 0);
}

/// parses the given token as float
static inline float jsmn_get_float(const char* js, jsmntok_t* token) {
    const char* offset = &(js[token->start]);
    return strtof(offset, NULL);
}


#endif //YADF2_JASMINE_H
