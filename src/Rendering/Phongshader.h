//
// Created by ieperen3039 on 12-12-19.
//

#ifndef YADF_PHONGSHADER_H
#define YADF_PHONGSHADER_H

#include <global.h>
#include <GL/glew.h>
#include "../DataStructures/Matrix4f.h"

typedef struct _Phongshader Phongshader;

/// create a new phong shader, should only be called once
PURE Phongshader* phong_create();

void phong_bind(Phongshader* shader);

void phong_unbind(Phongshader* shader);

void phong_set_model_matrix(Phongshader* shader, Matrix4f* matrix);

void phong_set_view_projection_matrix(Phongshader* shader, Matrix4f* matrix);

void phong_free(Phongshader* shader);

GLuint phong_id(Phongshader* shader);

void phong_add_light(Phongshader* shader, Vector3fc position, Color4f color, float intensity, bool infinite);

#endif //YADF_PHONGSHADER_H
