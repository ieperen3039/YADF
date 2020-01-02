//
// Created by s152717 on 29-12-2019.
//

#include "RockEntities.h"
#include "../Rendering/Mesh.h"
#include "../Rendering/Phongshader.h"

static Mesh* cube_mesh = NULL;

void slate_update(void* slate_data, WorldChunk* chunk, Vector3i* position) {
    // sit...
}

void slate_render(void* slate_data, Phongshader* shader) {
    if (!cube_mesh) cube_mesh = mesh_from_file("res/models/cube.obj");
    phong_set_material(shader, 0, MATERIAL_PROPERTIES_DEFAULT);
    mesh_render(cube_mesh);
}

void marble_update(void* marble_data, WorldChunk* chunk, Vector3i* position) {

}

void marble_render(void* marble_data, Phongshader* shader) {
    slate_render(NULL, shader);
}

