//
// Created by ieperen3039 on 12-12-19.
//

#include <assert.h>
#include "Phongshader.h"
#include "Shader.h"
#include "../Tools.h"
#include "../DataStructures/Matrix4f.h"
#include "../Materials.h"

static const Vector3fc AMBIENT_LIGHT = {0.15f, 0.15f, 0.15f};

struct _Phongshader {
    ShaderID ID;
    GLuint modelMatrix; // matrix4f
    GLuint viewProjectionMatrix; // matrix4f
    GLuint normalMatrix; // matrix3f
    GLuint cameraPosition; // vec3f
    GLuint ambientLight; // vec3f

    int nextLightIndex;
    struct SLight {
        GLuint color;
        GLuint mPosition;
        GLuint intensity;
    } lights[PHONGSHADER_MAX_NUM_LIGHTS];
    
    struct SMaterial {
        GLuint diffuse;
        GLuint specular;
        GLuint reflectance;
    } materials[PHONGSHADER_MAX_NUM_MATERIALS];
};

Phongshader* phong_create() {
    char* vertex_shader = readFile("res/phong.vert", NULL);
    char* fragment_shader = readFile("res/phong.frag", NULL);
    if (vertex_shader == NULL || fragment_shader == NULL) return NULL;

    ShaderID shader = shader_create(vertex_shader, fragment_shader, NULL);
    if (!shader) return NULL;

    Phongshader* p = malloc(sizeof(Phongshader));
    p->ID = shader;
    p->modelMatrix = glGetUniformLocation(shader, "modelMatrix");
    p->normalMatrix = glGetUniformLocation(shader, "normalMatrix");
    p->viewProjectionMatrix = glGetUniformLocation(shader, "viewProjectionMatrix");
    p->ambientLight = glGetUniformLocation(shader, "ambientLight");
    p->cameraPosition = glGetUniformLocation(shader, "cameraPosition");

    for (int i = 0; i < PHONGSHADER_MAX_NUM_LIGHTS; ++i) {
        char uniform_name[25];
        int curr = snprintf(uniform_name, 12, "lights[%d]", i);
        char* field = uniform_name + curr;

        strcpy(field, ".color");
        p->lights[i].color = glGetUniformLocation(shader, uniform_name);
        strcpy(field, ".mPosition");
        p->lights[i].mPosition = glGetUniformLocation(shader, uniform_name);
        strcpy(field, ".intensity");
        p->lights[i].intensity = glGetUniformLocation(shader, uniform_name);
    }
    
    for (int i = 0; i < PHONGSHADER_MAX_NUM_MATERIALS; ++i) {
        char uniform_name[30];
        int curr = snprintf(uniform_name, 17, "materials[%d]", i);
        char* field = uniform_name + curr;

        strcpy(field, ".diffuse");
        p->materials[i].diffuse = glGetUniformLocation(shader, uniform_name);
        strcpy(field, ".specular");
        p->materials[i].specular = glGetUniformLocation(shader, uniform_name);
        strcpy(field, ".reflectance");
        p->materials[i].reflectance = glGetUniformLocation(shader, uniform_name);
    }

    return p;
}

void phong_bind(Phongshader* shader, Vector3fc* camera_eye) {
    glUseProgram(shader->ID);
    glUniform3f(shader->ambientLight, AMBIENT_LIGHT.x, AMBIENT_LIGHT.y, AMBIENT_LIGHT.z);
    glUniform3f(shader->cameraPosition, camera_eye->x, camera_eye->y, camera_eye->z);

    // set all lights to zero
    for (int i = 0; i < PHONGSHADER_MAX_NUM_LIGHTS; ++i) {
        glUniform1f(shader->lights[i].intensity, 0);
    }
    shader->nextLightIndex = 0;
}

void phong_unbind(Phongshader* shader) {
    glUseProgram(0);
}

void phong_add_light(Phongshader* shader, Vector3fc position, Color4f color, float intensity, bool infinite) {
    int lightNumber = shader->nextLightIndex++;
    assert(lightNumber < PHONGSHADER_MAX_NUM_LIGHTS);
    assert(color.a > 0);
    struct SLight li = shader->lights[lightNumber];

    glUniform4f(li.mPosition, position.x, position.y, position.z, infinite ? 0.0f : 1.0f);
    glUniform3f(li.color, color.r / color.a, color.g / color.a, color.b / color.a);
    glUniform1f(li.intensity, intensity);
}

void phong_set_material_a(Phongshader* shader, int index, Color4f diffuse, Color4f specular, float reflectance) {
    assert(index < PHONGSHADER_MAX_NUM_MATERIALS);
    struct SMaterial mat = shader->materials[index];

    glUniform4f(mat.diffuse, diffuse.r, diffuse.g, diffuse.b, diffuse.a);
    glUniform4f(mat.specular, specular.r, specular.g, specular.b, specular.a);
    glUniform1f(mat.reflectance, reflectance);
}

void phong_set_material(Phongshader* shader, int index, MaterialProperties mats) {
    phong_set_material_a(shader, index, mats.diffuse, mats.specular, mats.reflectance);
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
