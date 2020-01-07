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

// a 3-element coordinate
typedef struct {
    int x, y, z;
} Vector3i;
typedef const Vector3i Vector3ic;

// a 4-element color
typedef struct {
    float r, g, b, a;
} Color4f;

static const Color4f COLOR_BLACK = {0, 0, 0, 1};
static const Color4f COLOR_WHITE = {1, 1, 1, 1};
static const Color4f COLOR_GRAY = {0.5f, 0.5f, 0.5f, 1};

typedef int UpdateCycle;

#endif //YADF2_STRUCTS_H
