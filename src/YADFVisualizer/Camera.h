//
// Created by ieperen3039 on 04-12-19.
//

#include "../YADFEngine/DataStructures/Structs.h"
#include <stdlib.h>
#include <math.h>
#include "global.h"

#ifndef YADF_CAMERA_H
#define YADF_CAMERA_H

/// an isometric, rotatable viewpoint on the world.
typedef struct _Camera Camera;

Camera* camera_new(Vector3fc* focus);

/** sets the viewing angle and zoom fraction of this camera. Any parameter smaller than zero is ignored */
void camera_set_perspective(Camera* cam, float angle, float zoom);

/** sets the center of the camera */
void camera_set_focus(Camera* cam, Vector3fc* focus);

/** @return a vector towards the camera. This vector is not normalized. */
Vector3f camera_get_eye(Camera* cam);

/** the width of area displayed by this camera */
float camera_get_view_size(Camera* cam);

/** @return the view transformation of the camera */
PURE Matrix4f camera_get_transform(Camera* cam);

#endif //YADF_CAMERA_H
