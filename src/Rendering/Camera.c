//
// Created by ieperen3039 on 04-12-19.
//

#include "Camera.h"

struct _Camera {
    Vector3f focus;
    float zoom;
    unsigned char rotation;
};

Camera* camera_new(Vector3fc* focus) {
    Camera* this = malloc(sizeof(Camera));
    this->focus = *focus; // copy
    this->rotation = 0;
    this->zoom = 0.01f;
    return this;
}

float camera_get_view_width(Camera* cam) {
    return 1.0f / cam->zoom;
}

Matrix4f camera_get_transform(Camera* cam) {
    float centerX = cam->focus.x;
    float centerY = cam->focus.y;
    float centerZ = cam->focus.z;
    // Compute direction from position to lookAt
    float dirX = (float) ((cam->rotation % 1) > 0) ? -sqrtf(3) : sqrtf(3);
    float dirY = (float) ((cam->rotation % 2) > 0) ? -sqrtf(3) : sqrtf(3);
    float dirZ = (float) -sqrtf(3);

    // left = up x direction
    float leftX = -dirY;
    float leftY = dirX;
    // normalize left
    float invLeftLength = 1.0f / (float) sqrtf(leftX * leftX + leftY * leftY);
    leftX *= invLeftLength;
    leftY *= invLeftLength;
    // up = direction x left
    float upnX = -dirZ * leftY;
    float upnY = dirZ * leftX;
    float upnZ = dirX * leftY - dirY * leftX;

    Matrix4f this = {
            {
                    {leftX, upnX, dirX, 0.0f},
                    {leftY, upnY, dirY, 0.0f},
                    {0.0f, upnZ, dirZ, 0.0f},
                    {
                            -(leftX * (centerX + 1) + leftY * (centerY + 1)),
                            -(upnX * (centerX + 1) + upnY * (centerY + 1) + upnZ * (centerZ + 1)),
                            -(dirX * (centerX + 1) + dirY * (centerY + 1) + dirZ * (centerZ + 1)),
                            (1.0f)
                    }
            }, (PROPERTY_AFFINE | PROPERTY_ORTHONORMAL)
    };
    return this;
}