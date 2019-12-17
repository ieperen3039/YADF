//
// Created by ieperen3039 on 29-11-19.
//

#ifndef YADF2_RENDER_H
#define YADF2_RENDER_H

#include "Camera.h"
#include "../DataStructures/Matrix4f.h"
#include "../DataStructures/Vector3f.h"
#include "../World/World.h"
#include "Mesh.h"
#include "Phongshader.h"

static void render_frame(Phongshader* shader, World* world, Camera* camera, float aspect_ratio) {
    phong_bind(shader);
    
//    float w = camera_get_view_width(camera);
    float w = 10;
    Matrix4f viewProjection = matrix_get_ortho_projection(w, w * aspect_ratio, 1, 100);
    Matrix4f viewTransform = camera_get_transform(camera);
    matrix_mul(&viewProjection, &viewTransform, &viewProjection);
    phong_set_view_projection_matrix(shader, &viewProjection);

    phong_add_light(shader, (Vector3f){1, 1, 1}, (Color4f){1, 1, 1, 1}, 1, true);

//    world_render_layer(world, 0);
    static Mesh* cubeMesh = NULL;
    if (!cubeMesh) cubeMesh = mesh_from_file("res/models/cube.obj");

    // write transform to gl
    Matrix4f model_matrix = matrix_get_identity();
    phong_set_model_matrix(shader, &model_matrix);
    mesh_render(cubeMesh);

    phong_unbind(shader);
}

#endif //YADF2_RENDER_H
