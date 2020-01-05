//
// Created by s152717 on 4-1-2020.
//

#include "Visual.h"
#include "Shader.h"
#include "Mesh.h"

struct _Visual {
    Mesh* mesh;
};

Visual* visual_create(const char* entity_name) {
    char filename[128];
    snprintf(filename, 128, "res/models/%s.obj", entity_name);
    Mesh* p_mesh = mesh_from_obj(filename, 1);

    Visual* v = malloc(sizeof(Visual*));
    v->mesh = p_mesh;

    return v;
}

void visual_draw(Visual* element, Shader* visualizer) {
    mesh_render(element->mesh);
}

void visual_free(Visual* element) {
    mesh_free(element->mesh);
    free(element);
}
