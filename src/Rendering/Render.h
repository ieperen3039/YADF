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
    static float cam_angle = 0;
    cam_angle += 0.01f;
    camera_set_perspective(camera, cam_angle, 0.05f);
    Vector3f eye = camera_get_eye(camera);
    char str[64];
    vector_to_string(&eye, str, 64);
    LOG_INFO(str);
    phong_bind(shader, &eye);
    
    float w = camera_get_view_width(camera);
    Matrix4f viewProjection = matrix_get_ortho_projection(w, w / aspect_ratio, -100, 100);
    Matrix4f viewTransform = camera_get_transform(camera);
    matrix_mul(&viewProjection, &viewTransform, &viewProjection);
    phong_set_view_projection_matrix(shader, &viewProjection);

    phong_add_light(shader, (Vector3f){2, 1, 3}, (Color4f){1, 1, 1, 1}, 1, true);
    phong_set_material(shader, 0, material_properties_default);

    // the model matrix
    Matrix4f model_matrix = matrix_get_identity();

//    world_render_layer(world, 0);
    static Mesh* cubeMesh = NULL;
    if (!cubeMesh) cubeMesh = mesh_from_file("res/models/cube.obj");

    phong_set_model_matrix(shader, &model_matrix);
    mesh_render(cubeMesh);

    phong_unbind(shader);
}

#endif //YADF2_RENDER_H
