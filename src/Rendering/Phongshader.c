//
// Created by ieperen3039 on 12-12-19.
//

#include <assert.h>
#include "Phongshader.h"
#include "Shader.h"
#include "../Tools.h"
#include "../DataStructures/Matrix4f.h"

#define MAX_NUM_LIGHTS 16

struct _Phongshader {
    ShaderID ID;
    GLuint modelMatrix; // matrix4f
    GLuint viewProjectionMatrix; // matrix4f
    GLuint normalMatrix; // matrix3f

    int nextLightIndex;
    struct Light {
        GLuint color;
        GLuint mPosition;
        GLuint intensity;
    } lights[MAX_NUM_LIGHTS];
};

Phongshader* phong_create() {
    char* vertex_shader = readFile("res/phong.vert", NULL);
    char* fragment_shader = readFile("res/phong.frag", NULL);
    if (vertex_shader == NULL || fragment_shader == NULL) return NULL;

    ShaderID shader = shader_create(vertex_shader, fragment_shader, NULL);

    Phongshader* p = malloc(sizeof(Phongshader));
    p->ID = shader;
    p->modelMatrix = glGetUniformLocation(shader, "modelMatrix");
    p->normalMatrix = glGetUniformLocation(shader, "normalMatrix");
    p->viewProjectionMatrix = glGetUniformLocation(shader, "viewProjectionMatrix");

    for (int i = 0; i < MAX_NUM_LIGHTS; ++i) {
        char uniform_name[22];
        int curr = snprintf(uniform_name, 12, "lights[%d]", i);
        char* field = uniform_name + curr;

        strcpy(field, ".color");
        p->lights[i].color = glGetUniformLocation(shader, uniform_name);
        strcpy(field, ".mPosition");
        p->lights[i].mPosition = glGetUniformLocation(shader, uniform_name);
        strcpy(field, ".intensity");
        p->lights[i].intensity = glGetUniformLocation(shader, uniform_name);
    }

    return p;
}

void phong_bind(Phongshader* shader) {
    glUseProgram(shader->ID);

    // set all lights to zero
    for (int i = 0; i < MAX_NUM_LIGHTS; ++i) {
        glUniform1f(shader->lights[i].intensity, 0);
    }
    shader->nextLightIndex = 0;
}

void phong_add_light(Phongshader* shader, Vector3fc position, Color4f color, float intensity, bool infinite) {
    int lightNumber = shader->nextLightIndex++;
    assert(lightNumber < MAX_NUM_LIGHTS);
    struct Light li = shader->lights[lightNumber];

    glUniform4f(li.mPosition, position.x, position.y, position.z, !infinite);
    glUniform3f(li.color, color.r / color.a, color.g / color.a, color.b / color.a);
    glUniform1f(li.intensity, intensity);
}

void phong_unbind(Phongshader* shader) {
    glUseProgram(0);
}

void phong_set_model_matrix(Phongshader* shader, Matrix4f* matrix) {
    glUniformMatrix4fv(shader->modelMatrix, 1, false, matrix_as_array(matrix));
    Matrix4f normal_mat;
    matrix_get_normal(matrix, &normal_mat);
    float normal_array[9];
    matrix_get_upper_left(&normal_mat, normal_array);
    glUniformMatrix3fv(shader->normalMatrix, 1, false, normal_array);
}

void phong_set_view_projection_matrix(Phongshader* shader, Matrix4f* matrix) {
    glUniformMatrix4fv(shader->viewProjectionMatrix, 1, false, matrix_as_array(matrix));
}

void phong_free(Phongshader* shader) {
    glDeleteProgram(shader->ID);
    free(shader);
}

GLuint phong_id(Phongshader* shader) { return shader->ID; }
