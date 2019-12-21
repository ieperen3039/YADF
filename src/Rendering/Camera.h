//
// Created by ieperen3039 on 04-12-19.
//

#include "../DataStructures/Vector3f.h"
#include "../DataStructures/Matrix4f.h"
#include <stdlib.h>
#include <math.h>

#ifndef YADF_CAMERA_H
#define YADF_CAMERA_H

typedef struct _Camera Camera;

Camera* camera_new(Vector3fc* focus);

void camera_set_perspective(Camera* cam, float angle, float zoom);

void camera_set_focus(Camera* cam, Vector3fc* focus);

Vector3f camera_get_eye(Camera* cam);

float camera_get_view_width(Camera* cam);

/** @return the view transformation of the camera */
PURE Matrix4f camera_get_transform(Camera* cam);

#endif //YADF_CAMERA_H
