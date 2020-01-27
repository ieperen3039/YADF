//
// Created by ieperen3039 on 12-12-19.
//

#ifndef YADF_SHADER_H
#define YADF_SHADER_H

#include "Structs.h"
#include "Materials.h"

#define SPRITE_HEIGHT_PIXELS 64.0
#define SPRITE_WIDTH_PIXELS 50.0

typedef struct _Shader Shader;

Shader* shader_new();

void shader_bind(Shader* shader, Vector3fc* eye, int window_width, int window_height);

void shader_unbind(Shader* shader);

/**
 * sets the ith part of the next object to the given material.
 * @param shader the current shader
 * @param index the part to color
 * @param material the material
 */
void shader_set_color(Shader* shader, int index, Color4f color);

void shader_set_tile_position(Shader* shader, Vector3i coordinate);

void shader_free(Shader* shader);

#endif //YADF_SHADER_H
