//
// Created by s152717 on 4-1-2020.
//

#ifndef YADF_VISUAL_H
#define YADF_VISUAL_H

typedef struct _Shader Shader;
typedef struct _Visual Visual;

Visual* visual_create(const char* entity_name);

void visual_draw(Visual* element, Shader* visualizer);

void visual_free(Visual* element);

#endif //YADF_VISUAL_H
