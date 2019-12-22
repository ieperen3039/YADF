//
// Created by ieperen3039 on 04-12-19.
//

#include "Camera.h"

#include "../DataStructures/Vector3f.h"
#include "../DataStructures/Matrix4f.h"

struct _Camera {
    Vector3f focus;
    float zoom;
    float theta; // rotation angle
};

Camera* camera_new(Vector3fc* focus) {
    Camera* this = malloc(sizeof(Camera));
    this->focus = *focus; // copy
    this->theta = 1;
    this->zoom = 0.05f;
    return this;
}

float camera_get_view_width(Camera* cam) {
    return 1.0f / cam->zoom;
}

void camera_set_perspective(Camera* cam, float angle, float zoom) {
    if (angle > 0) cam->theta = angle;
    if (zoom > 0) cam->zoom = zoom;
}

void camera_set_focus(Camera* cam, Vector3fc* focus) {
    cam->focus = *focus;
}

PURE Matrix4f camera_get_transform(Camera* cam) {
    const float centerX = cam->focus.x;
    const float centerY = cam->focus.y;
    const float centerZ = cam->focus.z;
    const float upX = 0;
    const float upY = 0;
    const float upZ = 1;

    float dirX = cosf(cam->theta);
    float dirY = sinf(cam->theta);
    float dirZ = 1;
    // normalize direction
    float invDirLength = 1.0f / sqrtf(dirX * dirX + dirY * dirY + dirZ * dirZ);
    dirX *= invDirLength;
    dirY *= invDirLength;
    dirZ *= invDirLength;

    // left = up x direction
    float leftX = upY * dirZ - upZ * dirY;
    float leftY = upZ * dirX - upX * dirZ;
    float leftZ = upX * dirY - upY * dirX;
    // normalize left
    float invLeftLength = 1.0f / sqrtf(leftX * leftX + leftY * leftY + leftZ * leftZ);
    leftX *= invLeftLength;
    leftY *= invLeftLength;
    leftZ *= invLeftLength;
    // up = direction x left
    float upnX = dirY * leftZ - dirZ * leftY;
    float upnY = dirZ * leftX - dirX * leftZ;
    float upnZ = dirX * leftY - dirY * leftX;
    float eyeX = centerX + dirX;
    float eyeY = centerY + dirY;
    float eyeZ = centerZ + dirZ;

    Matrix4f this = {
            .m = {
                    {leftX, upnX, dirX, 0.0f},
                    {leftY, upnY, dirY, 0.0f},
                    {leftZ, upnZ, dirZ, 0.0f},
                    {
                            -(leftX * eyeX + leftY * eyeY + leftZ * eyeZ),
                            -(upnX * eyeX + upnY * eyeY + upnZ * eyeZ),
                            -(dirX * eyeX + dirY * eyeY + dirZ * eyeZ),
                            (1.0f)
                    }
            },
            .properties = (PROPERTY_AFFINE | PROPERTY_ORTHONORMAL)
    };

    return this;

}

Vector3f camera_get_eye(Camera* cam) {
    float angle = cam->theta;
    return (Vector3f) {cosf(angle), sinf(angle), 1};
}
