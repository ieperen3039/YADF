//
// Created by ieperen3039 on 27-11-19.
//

#ifndef YADF2_STRUCTS_H
#define YADF2_STRUCTS_H

// a 4x4 float matrix
typedef struct {
    float m[4][4];
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
typedef const Vector3i Vector3ic;

typedef struct {
    const float r, g, b, a;
} Color4f;

#endif //YADF2_STRUCTS_H
