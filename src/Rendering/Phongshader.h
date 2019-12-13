//
// Created by ieperen3039 on 12-12-19.
//

#ifndef YADF_PHONGSHADER_H
#define YADF_PHONGSHADER_H

typedef struct _Phongshader Phongshader;

/// create a new phong shader, should only be called once
PURE Phongshader* phong_create();

void phong_bind(Phongshader* shader);

void phong_unbind(Phongshader* shader);

void phong_set_model_matrix(Phongshader* shader, float matrix[16]);

void phong_set_view_projection_matrix(Phongshader* shader, float matrix[16]);

void phong_set_normal_matrix(Phongshader* shader, float matrix[9]);

void phong_free(Phongshader* shader);

GLuint phong_id(Phongshader* shader);

#endif //YADF_PHONGSHADER_H
