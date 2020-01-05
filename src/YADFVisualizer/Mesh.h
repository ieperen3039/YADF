//
// Created by ieperen3039 on 28-11-19.
//

#ifndef YADF2_MESH_H
#define YADF2_MESH_H

#include <GL/glew.h>
#include "../YADFEngine/DataStructures/Map.h"

typedef struct _Mesh Mesh;

/**
 * create a mesh and write it to the GPU. For both lists it holds that the ith vertex has the ith position, the ith normal vector and the ith material
 * @param positions the vertex positions, concatenated in groups of 3 floats
 * @param normals   the vertex normals, concatenated in groups of 3 floats
 * @param materials the vertex materials, for each vertex one.
 * @param num_vertices the number of vertices
 */
Mesh* mesh_from_arrays(float* positions, float* normals, int* materials, int num_vertices);

/**
 * create a mesh from an obj file, and write it to the GPU.
 * @param filename the file path, relative to the exe file
 * @param material_id the material id for this whole object to use
 * @return an initialized mesh
 */
Mesh* mesh_from_obj(const char* filename, int material_id);

/**
 * create a mesh from a ply file, and write it to the GPU.
 * @param file_name the file path, relative to the exe file
 * @param color_map a map that takes colors and maps them to material indices, as used in the shader
 * @return
 */
Mesh* mesh_from_ply(const char* file_name, ElementMap* color_map);

void mesh_render(Mesh* mesh);

void mesh_free(Mesh* mesh);

#endif //YADF2_MESH_H
