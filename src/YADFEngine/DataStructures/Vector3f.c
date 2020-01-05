//
// Created by ieperen3039 on 27-11-19.
//
#include "Vector3f.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

Vector3f* vector_copy_to(Vector3fc* source, Vector3f* dest) {
    memcpy(dest, source, sizeof(Vector3f));
    return dest;
}

Vector3f* vector_set_zero(Vector3f* target) {
    memcpy(target, &VECTOR_ZERO, sizeof(Vector3f));
    return target;
}

Vector3f* vector_mul_position(Vector3f* vec, const Matrix4f* mat) {
    float x = mat->m[0][0] * vec->x + mat->m[1][0] * vec->y + mat->m[2][0] * vec->z + mat->m[3][0];
    float y = mat->m[0][1] * vec->x + mat->m[1][1] * vec->y + mat->m[2][1] * vec->z + mat->m[3][1];
    float z = mat->m[0][2] * vec->x + mat->m[1][2] * vec->y + mat->m[2][2] * vec->z + mat->m[3][2];
    vec->x = x;
    vec->y = y;
    vec->z = z;
    return vec;
}

Vector3f* vector_mul_direction(Vector3f* vec, const Matrix4f* mat) {
    float x = mat->m[0][0] * vec->x + mat->m[1][0] * vec->y + mat->m[2][0] * vec->z;
    float y = mat->m[0][1] * vec->x + mat->m[1][1] * vec->y + mat->m[2][1] * vec->z;
    float z = mat->m[0][2] * vec->x + mat->m[1][2] * vec->y + mat->m[2][2] * vec->z;
    vec->x = x;
    vec->y = y;
    vec->z = z;
    return vec;
}

float vector_dot(Vector3fc* this, Vector3fc* other) {
    return this->x * other->x + this->y * other->y + this->z * other->z;
}

Vector3f* vector_cross(Vector3f* vec, Vector3fc* other) {
    float x = vec->y * other->z - vec->z * other->y;
    float y = vec->z * other->x - vec->x * other->z;
    float z = vec->x * other->y - vec->y * other->x;
    vec->x = x;
    vec->y = y;
    vec->z = z;
    return vec;
}

Vector3f* vector_normalize(Vector3f* vec) {
    float invLength = 1.0f / vector_length_sq(vec);
    vec->x = vec->x * invLength;
    vec->y = vec->y * invLength;
    vec->z = vec->z * invLength;
    return vec;
}

Vector3f* vector_mul(Vector3f* vec, float scalar) {
    vec->x = vec->x * scalar;
    vec->y = vec->y * scalar;
    vec->z = vec->z * scalar;
    return vec;
}

Vector3f* vector_copy(Vector3fc* vec) {
    Vector3f* copy = malloc(sizeof(Vector3f));
    return vector_copy_to(vec, copy);
}

float vector_length_sq(Vector3fc* vec) {
    float x = vec->x;
    float y = vec->y;
    float z = vec->z;
    return (x * x + y * y + z * z);
}

float vector_length(Vector3fc* vec) {
    return sqrtf(vector_length_sq(vec));
}
