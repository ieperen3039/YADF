//
// Created by ieperen3039 on 29-11-19.
//

#ifndef YADF2_RENDER_H
#define YADF2_RENDER_H

#include "Shader.h"
#include "Camera.h"
#include "../DataStructures/Matrix4f.h"
#include "../DataStructures/Vector3f.h"
#include "../world.h"
#include "Mesh.h"

static void render_frame(ShaderID shader, World* world, Camera* camera, float aspect_ratio) {
    shader_bind(shader);
    
//    float w = camera_get_view_width(camera);
    float w = 10;

    Matrix4f viewProjection = matrix_get_ortho_projection(w, w * aspect_ratio, 0, w);
    Matrix4f viewTransform = camera_get_transform(camera);
    matrix_mul(&viewProjection, &viewTransform, &viewProjection);


    uniform mat4 modelMatrix;
    uniform mat4 viewProjectionMatrix;
    uniform mat3 normalMatrix;

//    world_render_layer(world, 0);
    Mesh* cubeMesh = mesh_from_file("res/models/cube.obj");
    mesh_render(cubeMesh, viewProjection, shader);

    shader_unbind();
}

struct {

};

#endif //YADF2_RENDER_H
