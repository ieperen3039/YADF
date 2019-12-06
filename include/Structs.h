//
// Created by ieperen3039 on 27-11-19.
//

#ifndef YADF2_STRUCTS_H
#define YADF2_STRUCTS_H

// a 4x4 float matrix
typedef struct {
    float m00, m01, m02, m03;
    float m10, m11, m12, m13;
    float m20, m21, m22, m23;
    float m30, m31, m32, m33;
    unsigned char properties;
} Matrix4f;
typedef const Matrix4f Matrix4fc;

// a 3-element float vector
typedef struct {
    float x, y, z;
} Vector3f;
typedef const Vector3f Vector3fc;

typedef struct {
    int x, y, z;
} Vector3i;

#endif //YADF2_STRUCTS_H
