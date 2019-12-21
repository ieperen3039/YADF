//
// Created by ieperen3039 on 27-11-19.
//

#ifndef YADF2_VECTOR3F_H
#define YADF2_VECTOR3F_H

#include "Structs.h"

#include <stdio.h>

static Vector3fc VECTOR_ZERO = {0, 0, 0};
static Vector3fc VECTOR_X = {1, 0, 0};
static Vector3fc VECTOR_Y = {0, 1, 0};
static Vector3fc VECTOR_Z = {0, 0, 1};

/**
 * Scale this vector to have length 1 and store the result in <code>dest</code>.
 *
 * @param vec the vector to normalize
 * @param vec will hold the result
 * @return dest
 */
Vector3f* vector_normalize(Vector3f* vec);

/**
 * allocate a new vector on the heap, and initialize it with the given vector
 * @param vec
 * @return a newly allocated vector
 */
Vector3f* vector_copy(Vector3fc* vec);

/**
 * copies `source` into `dest`
 * @param source the vector to read from
 * @param dest will hold the result
 * @return dest
 */
Vector3f* vector_copy_to(Vector3fc* source, Vector3f* dest);

/**
 * sets target to (0, 0, 0)
 * @param target
 * @return target
 */
Vector3f* vector_set_zero(Vector3f* target);

/**
 * scale the given vector with the given scalar
 * @param vec a vector
 * @param scalar a scalar multiplier
 * @param vec will hold the result
 * @return dest
 */
Vector3f* vector_mul(Vector3f* vec, float scalar);

/**
 * Multiply `vec` with the given 4x4 matrix <code>mat</code>.
 * <p>
 * This method assumes the <code>w</code> component of <code>vec</code> to be <code>1.0</code>.
 *
 * @param vec a vector, will hold the result
 * @param mat the matrix to multiply with
 * @return vec
 */
Vector3f* vector_mul_position(Vector3f* vec, const Matrix4f* mat);

/**
 * Multiply `vec` with the given 4x4 matrix <code>mat</code>.
 * <p>
 * This method assumes the <code>w</code> component of <code>vec</code> to be <code>0.0</code>.
 *
 * @param vec a vector
 * @param mat the matrix to multiply with
 * @param vec will hold the result
 * @return dest
 */
Vector3f* vector_mul_direction(Vector3f* vec, const Matrix4f* mat);

/**
 * Return the dot product of this vector and the supplied vector.
 * @param this one vector
 * @param other another vector
 * @return the dot product
 */
float vector_dot(Vector3fc* this, Vector3fc* other);

/**
 * Compute the cross product of this vector and <code>v</code> and store the result in <code>dest</code>.
 * @param vec one vector
 * @param other another vector
 * @param vec will hold the result
 * @return dest
 */
Vector3f* vector_cross(Vector3f* vec, Vector3fc* other);

float vector_length_sq(Vector3fc* vec);

float vector_length(Vector3fc* vec);

/** 
 * If dest is not null and the string representation of vec is smaller than str_size,
 * then the string representation of vector vec is written into dest.
 * Returns the size of the buffer needed to fit the string including null-delimiter.
 * This function behaves identical to snprintf(char*, int, const char*, ...).
 * 
 * To get a stack-allocated string, one can use the following snippet:
 * @code
 * Vector4f vec;
 * int length = vector_to_string(vec, NULL, 0);
 * char str[length + 1];
 * vector_to_string(vec, str, length + 1);
 * // str contains [vec\0]
 * @endcode
 *
 * To print multiple vertices:
 * @code
 * Vector4f vec1, vec2;
 * int length1 = vector_to_string(vec1, NULL, 0);
 * int length2 = vector_to_string(vec2, NULL, 0);
 * char str[length1 - 1 + length2];
 * int loc = vector_to_string(vec, str, length1 - 1 + length2); // loc == length1
 * vector_to_string(vec2, str + loc - 1, length2);
 * // str contains [vec1vec2\0]
 * @endcode
 *
 * @return the size of the string representation of vec, including null-delimiter.
 */
static inline int vector_to_string(Vector3fc* vec, char* dest, int str_size) {
    return snprintf(dest, str_size,
                   "(%6.03f, %6.03f, %6.03f)",
                   vec->x, vec->y, vec->z
    );
}

#endif //YADF2_VECTOR3F_H
