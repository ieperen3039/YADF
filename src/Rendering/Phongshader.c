//
// Created by ieperen3039 on 12-12-19.
//

#include "Phongshader.h"
#include "Shader.h"
#include "../Tools.c"

struct _Phongshader {
    ShaderID ID;
    GLuint modelMatrix; // matrix4f
    GLuint viewProjectionMatrix; // matrix4f
    GLuint normalMatrix; // matrix3f
};

Phongshader* phong_create(Allocator alloc) {
    char* vertex_shader = readFile("res/phong.vert", NULL);
    char* fragment_shader = readFile("res/phong.frag", NULL);
    ShaderID shader = shader_create(vertex_shader, fragment_shader, NULL);

    Phongshader* p = alloc.get(&alloc, sizeof(Phongshader));
    p->ID = shader;
    p->modelMatrix = glGetUniformLocation(shader, "modelMatrix");
    p->normalMatrix = glGetUniformLocation(shader, "normalMatrix");
    p->viewProjectionMatrix = glGetUniformLocation(shader, "viewProjectionMatrix");

    return p;
}

void phong_bind(Phongshader* shader) {
    glUseProgram(shader->ID);
}

void phong_unbind(Phongshader* shader) {
    glUseProgram(0);
}

void phong_set_model_matrix(Phongshader* shader, float* matrix) {
    glUniformMatrix4fv(shader->modelMatrix, 1, false, matrix);
}

void phong_set_view_projection_matrix(Phongshader* shader, float* matrix) {
    glUniformMatrix4fv(shader->viewProjectionMatrix, 1, false, matrix);
}

void phong_set_normal_matrix(Phongshader* shader, float* matrix) {
    glUniformMatrix3fv(shader->normalMatrix, 1, false, matrix);
}

void phong_free(Phongshader* shader) {
    shader_free(shader->ID);
}

GLuint phong_id(Phongshader* shader) { return shader.ID; }
