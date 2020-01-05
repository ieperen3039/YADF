//
// Created by s152717 on 3-1-2020.
//

#include "MaterialProperties.h"
#include "../YADFEngine/Tools.h"
#include "../YADFEngine/External/cJSON.h"
#include "../YADFEngine/DataStructures/Map.h"

ErrorCode material_read_json(const char* file, Color4f* material_colors) {
    int file_length;
    char* file_string = tool_read_file(file, &file_length);
    if (!file_string) return ERROR_IO;

    tool_strip_char(file_string, file_length, ' ');

    cJSON* json = cJSON_Parse(file_string);
    if (!json) {
        const char* error_point = cJSON_GetErrorPtr();
        char msg[21];
        memcpy(msg, error_point, 20);
        msg[20] = '\0';

        LOG_ERROR_F("Failed to read json file. Error occurred while reading '%s'", msg);
        return ERROR_IO;
    }

    cJSON* j_material = cJSON_GetObjectItem(json, "materials");
    if (!j_material){
        LOG_ERROR("json file did not contain a 'materials' field");
    }

    ElementMap materialColorMap;
    tool_read_color_map(j_material, &materialColorMap);
    
    MapIterator itr = map_iterator(&materialColorMap);
    while (map_iterator_has_next(&itr)) {
        struct MapIteratorPair pair = map_iterator_next(&itr);

        unsigned int color_hex = pair.key;
        unsigned char r = (color_hex >> 0 ) & 0xFF;
        unsigned char g = (color_hex >> 8 ) & 0xFF;
        unsigned char b = (color_hex >> 16) & 0xFF;
        Color4f color = {r / 255.0f, g / 255.0f, b / 255.0f, 1};

        const char** name = pair.value;

        bool found = false;
        for (int i = 0; i < MaterialSize; ++i) {
            Material mat = MaterialValues[i];

            if (strcmpi(material_name(mat), *name) == 0){
                material_colors[i] = color;
                found = true;
                break;
            }
        }

        if (!found){
            LOG_ERROR_F("Could not match %s with any of the known materials", *name);
        }
    }

    cJSON_Delete(json);
    map_free(&materialColorMap);
    return ERROR_NONE;
}
