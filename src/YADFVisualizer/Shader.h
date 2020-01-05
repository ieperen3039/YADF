//
// Created by ieperen3039 on 12-12-19.
//

#ifndef YADF_SHADER_H
#define YADF_SHADER_H

#include "../YADFEngine/DataStructures/Structs.h"
#include "../YADFEngine/Entities/Materials.h"

typedef struct _Shader Shader;

/**
 * sets the ith part of the next object to the given material.
 * @param shader the current shader
 * @param index the part to color
 * @param material the material
 */
void shader_set_material(Shader* shader, int index, Material material);

void shader_set_tile_position(Shader* shader, Vector3i coordinate);

void shader_set_tile_rotation(Shader* shader, char rotation);

void shader_set_view_projection_matrix(Shader* shader, Matrix4f* matrix);

void shader_add_light(Shader* shader, Vector3fc position, Color4f color, float intensity, bool infinite);

#endif //YADF_SHADER_H
