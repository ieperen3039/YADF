//
// Created by s152717 on 3-1-2020.
//

#include "Tools.h"
#include "External/cJSON.h"
#include "ctype.h"

char* tool_read_file(const char* filename, int* strlen) {
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

ErrorCode tool_read_color_map(cJSON* mapping, ElementMap* map) {
    map_init(map, sizeof(char*));

    cJSON* elt = mapping->child;

    while (elt != NULL) {
        unsigned int identifier;
        if (cJSON_IsArray(elt)) {
            unsigned char r = cJSON_GetArrayItem(elt, 0)->valueint & 0xFF;
            unsigned char g = cJSON_GetArrayItem(elt, 1)->valueint & 0xFF;
            unsigned char b = cJSON_GetArrayItem(elt, 2)->valueint & 0xFF;

            identifier = (r) + (g << 8) + (b << 16);

        } else {
            const char* hex = cJSON_GetStringValue(elt);
            if (!hex) {
                LOG_ERROR_F("Value of element %s is not an array, nor a string hex", cJSON_Print(elt));
                return ERROR_IO;
            }

            if (hex[0] == '#') hex++; // skip #
            identifier = strtol(hex, NULL, 16);
        }

        assert(!map_contains(map, identifier));
        map_insert(map, (int) identifier, &elt->string);

        elt = elt->next;
    }

    return ERROR_NONE;
}

void tool_strip_char(char* buffer, int strlen, char replacement) {
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
}
