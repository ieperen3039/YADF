//
// Created by s152717 on 5-1-2020.
//

#ifndef YADF_JSONUTIL_H
#define YADF_JSONUTIL_H

ErrorCode util_read_color_map(cJSON* mapping, ElementMap* map) {
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

#endif //YADF_JSONUTIL_H
