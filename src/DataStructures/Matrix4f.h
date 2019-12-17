//
// Bugs by ieperen3039
// Created on 27-11-19.
//

#ifndef YADF2_MATRIX4F_H
#define YADF2_MATRIX4F_H

#include "Structs.h"
#include <string.h>
#include <stdbool.h>
#include <global.h>

#define PROPERTY_IDENTITY 0x01
#define PROPERTY_AFFINE 0x02
#define PROPERTY_TRANSLATION 0x04
#define PROPERTY_ORTHONORMAL 0x08
#define PROPERTY_PERSPECTIVE 0x10

/**
 * @return a new identity matrix
 */
Matrix4f matrix_get_identity();

/**
 * Set this matrix to be an orthographic projection transformation for a right-handed coordinate system
 * using the OpenGL NDC z range [-1 ... 1].
 * <p>
 * Reference: <a href="http://www.songho.ca/opengl/gl_projectionmatrix.html#ortho">http://www.songho.ca</a>
 *
 * @param width the distance between the right and left frustum edges
 * @param height the distance between the top and bottom frustum edges
 * @param zNear near clipping plane distance
 * @param zFar far clipping plane distance
 * @return an orthographic projection matrix
 */
Matrix4f matrix_get_ortho_projection(float width, float height, float zNear, float zFar);

/**
 * Set this matrix to be an orthographic projection transformation for a right-handed coordinate system
 * using the Vulkan's and Direct3D's NDC z range of [0 ... 1]
 * <p>
 * Reference: <a href="http://www.songho.ca/opengl/gl_projectionmatrix.html#ortho">http://www.songho.ca</a>
 *
 * @param width the distance between the right and left frustum edges
 * @param height the distance between the top and bottom frustum edges
 * @param zNear near clipping plane distance
 * @param zFar far clipping plane distance
 * @return an orthographic projection matrix
 */
Matrix4f matrix_get_ortho_projection_unsigned(float width, float height, float zNear, float zFar);

/**
 * sets the given matrix to a normal matrix
 * @param dest will hold the result
 *  */
void matrix_set_identity(Matrix4f* dest);

/**
 * copies `source` into `dest`
 * @param source the matrix to read from
 * @param dest will hold the result
 *  */
void matrix_copy_to(Matrix4fc* source, Matrix4f* dest);

/**
 * multiplies the `first` matrix with `second` as (first * second)
 * @param first the left matrix
 * @param second the right matrix
 * @param dest will hold the result
 * @return
 */
void matrix_mul(Matrix4f* first, Matrix4f* second, Matrix4f* dest);

/**
 * Project the given `(x, y, z)` position via `this` matrix using the specified viewport
 * and store the resulting window coordinates in `winCoordsDest`.
 * <p>
 * This method transforms the given coordinates by `this` matrix including perspective division to
 * obtain normalized device coordinates, and then translates these into window coordinates by using the
 * given `viewport` settings `[x, y, width, height]`.
 * <p>
 * The depth range of the returned `winCoordsDest.z` will be `[0..1]`, which is also the OpenGL default.
 *
 * @param x the x-coordinate of the position to project
 * @param y the y-coordinate of the position to project
 * @param z the z-coordinate of the position to project
 * @param viewport the viewport described by [x, y, width, height]
 * @param winCoordsDest will hold the projected window coordinates
 * @return winCoordsDest
 */
Vector3f* matrix_project(Matrix4f* this, float x, float y, float z, const int viewport[4], Vector3f* winCoordsDest);

/**
 * Compute a normal matrix from the upper left 3x3 submatrix of <code>this</code>
 * and store it into the upper left 3x3 submatrix of <code>dest</code>.
 * All other values of <code>dest</code> will be set to {@link #identity() identity}.
 * <p>
 * The normal matrix of <code>m</code> is the transpose of the inverse of <code>m</code>.
 * <p>
 * Please note that, if <code>this</code> is an orthogonal matrix or a matrix whose columns are orthogonal vectors,
 * then this method <i>need not</i> be invoked, since in that case <code>this</code> itself is its normal matrix.
 * In that case, use {@link #set3x3(Matrix4f)} to set a given Matrix4f to only the upper left 3x3 submatrix
 * of this matrix.
 *
 * @see #set3x3(Matrix4f)
 *
 * @param dest
 *             will hold the result
 *  */
void matrix_get_normal(Matrix4f* this, Matrix4f* dest);

void matrix_translate(Matrix4fc* this, Vector3fc* translation, Matrix4f* dest);

/**
 * does a deep-compare of the two given matrices. This differs from (memcmp(one, two, sizeof(Matrix4f)) != 0) in that
 * the properties field is ignored.
 * @return true if one is effectively the same as two
 */
bool matrix_equals(Matrix4f* one, Matrix4f* two);

/** returns a pointer to the values in this matrix as an array of size 16 */
PURE static inline float* matrix_as_array(Matrix4f* mat) {
    return (float*) mat->m;
};

/** copies the upper left area of the matrix to the given array */
static inline void matrix_get_upper_left(Matrix4f* source, float dest[9]) {
    int i = 0;
    for (int u = 0; u < 3; ++u) {
        for (int v = 0; v < 3; ++v) {
            dest[i++] = source->m[u][v];
        }
    }
}

/** 
 * If dest is not null and the string representation of mat is smaller than str_size,
 * then the string representation of matrix mat is written into dest.
 * Returns the size of the buffer needed to fit the string including null-delimiter.
 * This function behaves identical to snprintf(char*, int, const char*, ...).
 * 
 * To get a stack-allocated string, one can use the following snippet:
 * @code
 * Matrix4f mat;
 * int length = matrix_to_string(mat, NULL, 0);
 * char str[length + 1];
 * matrix_to_string(mat, str, length + 1);
 * // str contains [mat\0]
 * @endcode
 *
 * To print multiple matrices:
 * @code
 * Matrix4f mat1, mat2;
 * int length1 = matrix_to_string(mat1, NULL, 0);
 * int length2 = matrix_to_string(mat2, NULL, 0);
 * char str[length1 - 1 + length2];
 * int loc = matrix_to_string(mat, str, length1 - 1 + length2); // loc == length1
 * matrix_to_string(mat2, str + loc - 1, length2);
 * // str contains [mat1mat2\0]
 * @endcode
 *
 * @return the size of the string representation of mat, including null-delimiter.
 */
static inline int matrix_to_string(Matrix4fc* mat, char* dest, int str_size) {
    return snprintf( // this took longer than it had to
            dest, str_size,
            "[[%6.03f, %6.03f, %6.03f, %6.03f][%6.03f, %6.03f, %6.03f, %6.03f][%6.03f, %6.03f, %6.03f, %6.03f][%6.03f, %6.03f, %6.03f, %6.03f]]",
            mat->m[0][0], mat->m[1][0], mat->m[2][0], mat->m[3][0],
            mat->m[0][1], mat->m[1][1], mat->m[2][1], mat->m[3][1],
            mat->m[0][2], mat->m[1][2], mat->m[2][2], mat->m[3][2],
            mat->m[0][3], mat->m[1][3], mat->m[2][3], mat->m[3][3]
    );
}


#endif //YADF2_MATRIX4F_H
