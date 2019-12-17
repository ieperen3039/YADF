//
// Created by ieperen3039 on 28-11-19.
//

#ifndef YADF2_MESH_H
#define YADF2_MESH_H

#include <GL/glew.h>
#include "../DataStructures/List.h"
#include "Structs.h"
#include "Shader.h"

typedef struct _Mesh Mesh;

/**
 * create a mesh and store it to the GPU. For both lists it holds that the ith vertex has the ith normal vector
 * @param positions the vertices, concatenated in groups of 3
 * @param normals   the normals, concatenated in groups of 3
 * @param colors    the vertex colors, concatenated in groups of 4. May be null.
 * @throws IllegalArgumentException if positions or normals has length not divisible by 3, or when colors has length
 *                                  not divisible by 4.
 * @throws IllegalArgumentException if the arrays are of unequal length
 * @throws IllegalStateException    if the mesh is already loaded
 */
Mesh* mesh_from_arrays(float* normals, float* positions, int num_vertices);

/**
 * create a mesh and store it to the GPU.
 * @param file_name
 * @return
 */
Mesh* mesh_from_file(const char* file_name);

void mesh_render(Mesh* mesh);

void mesh_free(Mesh* mesh);

#endif //YADF2_MESH_H
