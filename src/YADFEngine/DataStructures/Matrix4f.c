/*
 * The MIT License
 *
 * Copyright (c) 2015-2019 Richard Greenlees
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "Matrix4f.h"
#include <stdlib.h>

const Matrix4f IDENTITY = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        PROPERTY_IDENTITY | PROPERTY_AFFINE | PROPERTY_TRANSLATION | PROPERTY_ORTHONORMAL
};

void matrix_set_identity(Matrix4f* dest) {
    memcpy(dest, &IDENTITY, sizeof(Matrix4f));
}

void mulAffine(Matrix4f* first, Matrix4f* second, Matrix4f* dest) {
    float m00 = first->m[0][0] * second->m[0][0] + first->m[1][0] * second->m[0][1] + first->m[2][0] * second->m[0][2];
    float m01 = first->m[0][1] * second->m[0][0] + first->m[1][1] * second->m[0][1] + first->m[2][1] * second->m[0][2];
    float m02 = first->m[0][2] * second->m[0][0] + first->m[1][2] * second->m[0][1] + first->m[2][2] * second->m[0][2];
    float m03 = first->m[0][3];
    float m10 = first->m[0][0] * second->m[1][0] + first->m[1][0] * second->m[1][1] + first->m[2][0] * second->m[1][2];
    float m11 = first->m[0][1] * second->m[1][0] + first->m[1][1] * second->m[1][1] + first->m[2][1] * second->m[1][2];
    float m12 = first->m[0][2] * second->m[1][0] + first->m[1][2] * second->m[1][1] + first->m[2][2] * second->m[1][2];
    float m13 = first->m[1][3];
    float m20 = first->m[0][0] * second->m[2][0] + first->m[1][0] * second->m[2][1] + first->m[2][0] * second->m[2][2];
    float m21 = first->m[0][1] * second->m[2][0] + first->m[1][1] * second->m[2][1] + first->m[2][1] * second->m[2][2];
    float m22 = first->m[0][2] * second->m[2][0] + first->m[1][2] * second->m[2][1] + first->m[2][2] * second->m[2][2];
    float m23 = first->m[2][3];
    float m30 = first->m[0][0] * second->m[3][0] + first->m[1][0] * second->m[3][1] + first->m[2][0] * second->m[3][2] +
                first->m[3][0];
    float m31 = first->m[0][1] * second->m[3][0] + first->m[1][1] * second->m[3][1] + first->m[2][1] * second->m[3][2] +
                first->m[3][1];
    float m32 = first->m[0][2] * second->m[3][0] + first->m[1][2] * second->m[3][1] + first->m[2][2] * second->m[3][2] +
                first->m[3][2];
    // split for the case (dest == first || dest == second)
    dest->m[0][0] = m00;
    dest->m[0][1] = m01;
    dest->m[0][2] = m02;
    dest->m[0][3] = m03;
    dest->m[1][0] = m10;
    dest->m[1][1] = m11;
    dest->m[1][2] = m12;
    dest->m[1][3] = m13;
    dest->m[2][0] = m20;
    dest->m[2][1] = m21;
    dest->m[2][2] = m22;
    dest->m[2][3] = m23;
    dest->m[3][0] = m30;
    dest->m[3][1] = m31;
    dest->m[3][2] = m32;

    dest->properties = (PROPERTY_AFFINE | (first->properties & second->properties & PROPERTY_ORTHONORMAL));
}

void mulPerspectiveAffine(Matrix4f* first, Matrix4f* second, Matrix4f* dest) {
    float m00 = first->m[0][0] * second->m[0][0];
    float m01 = first->m[1][1] * second->m[0][1];
    float m02 = first->m[2][2] * second->m[0][2];
    float m03 = first->m[2][3] * second->m[0][2];
    float m10 = first->m[0][0] * second->m[1][0];
    float m11 = first->m[1][1] * second->m[1][1];
    float m12 = first->m[2][2] * second->m[1][2];
    float m13 = first->m[2][3] * second->m[1][2];
    float m20 = first->m[0][0] * second->m[2][0];
    float m21 = first->m[1][1] * second->m[2][1];
    float m22 = first->m[2][2] * second->m[2][2];
    float m23 = first->m[2][3] * second->m[2][2];
    float m30 = first->m[0][0] * second->m[3][0];
    float m31 = first->m[1][1] * second->m[3][1];
    float m32 = first->m[2][2] * second->m[3][2] + first->m[3][2];
    float m33 = first->m[2][3] * second->m[3][2];
    dest->m[0][0] = m00;
    dest->m[0][1] = m01;
    dest->m[0][2] = m02;
    dest->m[0][3] = m03;
    dest->m[1][0] = m10;
    dest->m[1][1] = m11;
    dest->m[1][2] = m12;
    dest->m[1][3] = m13;
    dest->m[2][0] = m20;
    dest->m[2][1] = m21;
    dest->m[2][2] = m22;
    dest->m[2][3] = m23;
    dest->m[3][0] = m30;
    dest->m[3][1] = m31;
    dest->m[3][2] = m32;
    dest->m[3][3] = m33;

    dest->properties = 0;
}

void mulAffineR(Matrix4f* first, Matrix4f* second, Matrix4f* dest) {
    float m00 = first->m[0][0] * second->m[0][0] + first->m[1][0] * second->m[0][1] + first->m[2][0] * second->m[0][2];
    float m01 = first->m[0][1] * second->m[0][0] + first->m[1][1] * second->m[0][1] + first->m[2][1] * second->m[0][2];
    float m02 = first->m[0][2] * second->m[0][0] + first->m[1][2] * second->m[0][1] + first->m[2][2] * second->m[0][2];
    float m03 = first->m[0][3] * second->m[0][0] + first->m[1][3] * second->m[0][1] + first->m[2][3] * second->m[0][2];
    float m10 = first->m[0][0] * second->m[1][0] + first->m[1][0] * second->m[1][1] + first->m[2][0] * second->m[1][2];
    float m11 = first->m[0][1] * second->m[1][0] + first->m[1][1] * second->m[1][1] + first->m[2][1] * second->m[1][2];
    float m12 = first->m[0][2] * second->m[1][0] + first->m[1][2] * second->m[1][1] + first->m[2][2] * second->m[1][2];
    float m13 = first->m[0][3] * second->m[1][0] + first->m[1][3] * second->m[1][1] + first->m[2][3] * second->m[1][2];
    float m20 = first->m[0][0] * second->m[2][0] + first->m[1][0] * second->m[2][1] + first->m[2][0] * second->m[2][2];
    float m21 = first->m[0][1] * second->m[2][0] + first->m[1][1] * second->m[2][1] + first->m[2][1] * second->m[2][2];
    float m22 = first->m[0][2] * second->m[2][0] + first->m[1][2] * second->m[2][1] + first->m[2][2] * second->m[2][2];
    float m23 = first->m[0][3] * second->m[2][0] + first->m[1][3] * second->m[2][1] + first->m[2][3] * second->m[2][2];
    float m30 = first->m[0][0] * second->m[3][0] + first->m[1][0] * second->m[3][1] + first->m[2][0] * second->m[3][2] +
                first->m[3][0];
    float m31 = first->m[0][1] * second->m[3][0] + first->m[1][1] * second->m[3][1] + first->m[2][1] * second->m[3][2] +
                first->m[3][1];
    float m32 = first->m[0][2] * second->m[3][0] + first->m[1][2] * second->m[3][1] + first->m[2][2] * second->m[3][2] +
                first->m[3][2];
    float m33 = first->m[0][3] * second->m[3][0] + first->m[1][3] * second->m[3][1] + first->m[2][3] * second->m[3][2] +
                first->m[3][3];

    dest->m[0][0] = m00;
    dest->m[0][1] = m01;
    dest->m[0][2] = m02;
    dest->m[0][3] = m03;
    dest->m[1][0] = m10;
    dest->m[1][1] = m11;
    dest->m[1][2] = m12;
    dest->m[1][3] = m13;
    dest->m[2][0] = m20;
    dest->m[2][1] = m21;
    dest->m[2][2] = m22;
    dest->m[2][3] = m23;
    dest->m[3][0] = m30;
    dest->m[3][1] = m31;
    dest->m[3][2] = m32;
    dest->m[3][3] = m33;
    dest->properties = (first->properties &
                        ~(PROPERTY_IDENTITY | PROPERTY_PERSPECTIVE | PROPERTY_TRANSLATION | PROPERTY_ORTHONORMAL));
}

void mulGeneric(Matrix4f* first, Matrix4f* second, Matrix4f* dest) {
    float m00 = first->m[0][0] * second->m[0][0] + first->m[1][0] * second->m[0][1] +
                first->m[2][0] * second->m[0][2] + first->m[3][0] * second->m[0][3];
    float m01 = first->m[0][1] * second->m[0][0] + first->m[1][1] * second->m[0][1] +
                first->m[2][1] * second->m[0][2] + first->m[3][1] * second->m[0][3];
    float m02 = first->m[0][2] * second->m[0][0] + first->m[1][2] * second->m[0][1] +
                first->m[2][2] * second->m[0][2] + first->m[3][2] * second->m[0][3];
    float m03 = first->m[0][3] * second->m[0][0] + first->m[1][3] * second->m[0][1] +
                first->m[2][3] * second->m[0][2] + first->m[3][3] * second->m[0][3];
    float m10 = first->m[0][0] * second->m[1][0] + first->m[1][0] * second->m[1][1] +
                first->m[2][0] * second->m[1][2] + first->m[3][0] * second->m[1][3];
    float m11 = first->m[0][1] * second->m[1][0] + first->m[1][1] * second->m[1][1] +
                first->m[2][1] * second->m[1][2] + first->m[3][1] * second->m[1][3];
    float m12 = first->m[0][2] * second->m[1][0] + first->m[1][2] * second->m[1][1] +
                first->m[2][2] * second->m[1][2] + first->m[3][2] * second->m[1][3];
    float m13 = first->m[0][3] * second->m[1][0] + first->m[1][3] * second->m[1][1] +
                first->m[2][3] * second->m[1][2] + first->m[3][3] * second->m[1][3];
    float m20 = first->m[0][0] * second->m[2][0] + first->m[1][0] * second->m[2][1] +
                first->m[2][0] * second->m[2][2] + first->m[3][0] * second->m[2][3];
    float m21 = first->m[0][1] * second->m[2][0] + first->m[1][1] * second->m[2][1] +
                first->m[2][1] * second->m[2][2] + first->m[3][1] * second->m[2][3];
    float m22 = first->m[0][2] * second->m[2][0] + first->m[1][2] * second->m[2][1] +
                first->m[2][2] * second->m[2][2] + first->m[3][2] * second->m[2][3];
    float m23 = first->m[0][3] * second->m[2][0] + first->m[1][3] * second->m[2][1] +
                first->m[2][3] * second->m[2][2] + first->m[3][3] * second->m[2][3];
    float m30 = first->m[0][0] * second->m[3][0] + first->m[1][0] * second->m[3][1] +
                first->m[2][0] * second->m[3][2] + first->m[3][0] * second->m[3][3];
    float m31 = first->m[0][1] * second->m[3][0] + first->m[1][1] * second->m[3][1] +
                first->m[2][1] * second->m[3][2] + first->m[3][1] * second->m[3][3];
    float m32 = first->m[0][2] * second->m[3][0] + first->m[1][2] * second->m[3][1] +
                first->m[2][2] * second->m[3][2] + first->m[3][2] * second->m[3][3];
    float m33 = first->m[0][3] * second->m[3][0] + first->m[1][3] * second->m[3][1] +
                first->m[2][3] * second->m[3][2] + first->m[3][3] * second->m[3][3];

    dest->m[0][0] = m00;
    dest->m[0][1] = m01;
    dest->m[0][2] = m02;
    dest->m[0][3] = m03;
    dest->m[1][0] = m10;
    dest->m[1][1] = m11;
    dest->m[1][2] = m12;
    dest->m[1][3] = m13;
    dest->m[2][0] = m20;
    dest->m[2][1] = m21;
    dest->m[2][2] = m22;
    dest->m[2][3] = m23;
    dest->m[3][0] = m30;
    dest->m[3][1] = m31;
    dest->m[3][2] = m32;
    dest->m[3][3] = m33;

    dest->properties = 0;
}

// consider adding function pointers into the struct
void matrix_mul(Matrix4f* first, Matrix4f* second, Matrix4f* dest) {
    if ((first->properties & PROPERTY_IDENTITY) != 0)
        matrix_copy_to(dest, second);
    else if ((second->properties & PROPERTY_IDENTITY) != 0)
        matrix_copy_to(dest, first);
    else if ((first->properties & PROPERTY_AFFINE) != 0 && (second->properties & PROPERTY_AFFINE) != 0)
        mulAffine(first, second, dest);
    else if ((first->properties & PROPERTY_PERSPECTIVE) != 0 && (second->properties & PROPERTY_AFFINE) != 0)
        mulPerspectiveAffine(first, second, dest);
    else if ((second->properties & PROPERTY_AFFINE) != 0)
        mulAffineR(first, second, dest);
    else
        mulGeneric(first, second, dest);
}

Matrix4f matrix_get_identity() {
    return IDENTITY; // creates a copy
}

Matrix4f* matrix_copy(Matrix4f* source) {
    Matrix4f* copy = malloc(sizeof(Matrix4f));
    matrix_copy_to(source, copy);
    return copy;
}

void matrix_copy_to(Matrix4fc* source, Matrix4f* dest) {
    if (source == dest) return;
    memcpy(dest, source, sizeof(Matrix4f));
}

Vector3f* matrix_project(Matrix4f* this, float x, float y, float z, const int* viewport, Vector3f* winCoordsDest) {
    float invW = 1.0f / (this->m[0][3] * x + this->m[1][3] * y + this->m[2][3] * z + this->m[3][3]);
    float nx = (this->m[0][0] * x + this->m[1][0] * y + this->m[2][0] * z + this->m[3][0]) * invW;
    float ny = (this->m[0][1] * x + this->m[1][1] * y + this->m[2][1] * z + this->m[3][1]) * invW;
    float nz = (this->m[0][2] * x + this->m[1][2] * y + this->m[2][2] * z + this->m[3][2]) * invW;
    winCoordsDest->x = (float) ((nx * 0.5 + 0.5) * viewport[2] + viewport[0]);
    winCoordsDest->y = (float) ((ny * 0.5 + 0.5) * viewport[3] + viewport[1]);
    winCoordsDest->z = (1.0f + nz) * 0.5f;
    return winCoordsDest;
}

Matrix4f matrix_get_ortho_projection(float width, float height, float zNear, float zFar) {
    Matrix4f this = matrix_get_identity();
    this.m[0][0] = (2.0f / width);
    this.m[1][1] = (2.0f / height);
    this.m[2][2] = (2.0f / (zNear - zFar));
    this.m[3][2] = ((zFar + zNear) / (zNear - zFar));
    this.properties = PROPERTY_AFFINE;
    return this;
}

Matrix4f matrix_get_ortho_projection_unsigned(float width, float height, float zNear, float zFar) {
    Matrix4f this = matrix_get_identity();
    this.m[0][0] = (2.0f / width);
    this.m[1][1] = (2.0f / height);
    this.m[2][2] = (1.0f / (zNear - zFar));
    this.m[3][2] = (zNear / (zNear - zFar));
    this.properties = PROPERTY_AFFINE;
    return this;
}

void normal_generic(const Matrix4f* this, Matrix4f* dest) {
    float m00m11 = this->m[0][0] * this->m[1][1];
    float m01m10 = this->m[0][1] * this->m[1][0];
    float m02m10 = this->m[0][2] * this->m[1][0];
    float m00m12 = this->m[0][0] * this->m[1][2];
    float m01m12 = this->m[0][1] * this->m[1][2];
    float m02m11 = this->m[0][2] * this->m[1][1];
    float det =
            (m00m11 - m01m10) * this->m[2][2] + (m02m10 - m00m12) * this->m[2][1] + (m01m12 - m02m11) * this->m[2][0];
    float s = 1.0f / det;
    /* Invert and transpose in one go */
    float nm00 = (this->m[1][1] * this->m[2][2] - this->m[2][1] * this->m[1][2]) * s;
    float nm01 = (this->m[2][0] * this->m[1][2] - this->m[1][0] * this->m[2][2]) * s;
    float nm02 = (this->m[1][0] * this->m[2][1] - this->m[2][0] * this->m[1][1]) * s;
    float nm10 = (this->m[2][1] * this->m[0][2] - this->m[0][1] * this->m[2][2]) * s;
    float nm11 = (this->m[0][0] * this->m[2][2] - this->m[2][0] * this->m[0][2]) * s;
    float nm12 = (this->m[2][0] * this->m[0][1] - this->m[0][0] * this->m[2][1]) * s;
    float nm20 = (m01m12 - m02m11) * s;
    float nm21 = (m02m10 - m00m12) * s;
    float nm22 = (m00m11 - m01m10) * s;

    dest->m[0][0] = (nm00);
    dest->m[0][1] = (nm01);
    dest->m[0][2] = (nm02);
    dest->m[0][3] = (0.0f);
    dest->m[1][0] = (nm10);
    dest->m[1][1] = (nm11);
    dest->m[1][2] = (nm12);
    dest->m[1][3] = (0.0f);
    dest->m[2][0] = (nm20);
    dest->m[2][1] = (nm21);
    dest->m[2][2] = (nm22);
    dest->m[2][3] = (0.0f);
    dest->m[3][0] = (0.0f);
    dest->m[3][1] = (0.0f);
    dest->m[3][2] = (0.0f);
    dest->m[3][3] = (1.0f);
    dest->properties = (this->properties | PROPERTY_AFFINE) & ~(PROPERTY_TRANSLATION | PROPERTY_PERSPECTIVE);
}

void matrix_get_normal(Matrix4f* this, Matrix4f* dest) {
    if ((this->properties & PROPERTY_IDENTITY) != 0) {
        matrix_set_identity(dest);

    } else if ((this->properties & PROPERTY_ORTHONORMAL) != 0) {
        matrix_copy_to(this, dest);

    } else {
        normal_generic(this, dest);
    }
}

void matrix_translate(Matrix4fc* this, Vector3fc* translation, Matrix4f* dest) {
    if (this != dest) {
        matrix_copy_to(this, dest);
    }
    float x = translation->x;
    float y = translation->y;
    float z = translation->z;

    dest->m[3][0] = (this->m[0][0] * x + this->m[1][0] * y + this->m[2][0] * z + this->m[3][0]);
    dest->m[3][1] = (this->m[0][1] * x + this->m[1][1] * y + this->m[2][1] * z + this->m[3][1]);
    dest->m[3][2] = (this->m[0][2] * x + this->m[1][2] * y + this->m[2][2] * z + this->m[3][2]);
    dest->m[3][3] = (this->m[0][3] * x + this->m[1][3] * y + this->m[2][3] * z + this->m[3][3]);
    dest->properties &= ~(PROPERTY_PERSPECTIVE | PROPERTY_IDENTITY);
}

void matrix_set_translation(Matrix4f* this, Vector3f translation) {
    this->m[3][0] = translation.x;
    this->m[3][1] = translation.y;
    this->m[3][2] = translation.z;
    this->properties &= ~(PROPERTY_PERSPECTIVE | PROPERTY_IDENTITY);
}

bool matrix_equals(Matrix4f* one, Matrix4f* two) {
    return memcmp(one->m, two->m, sizeof(one->m)) == 0;
}

void matrix_get_upper_left(Matrix4f* source, float* dest) {
    int i = 0;
    for (int u = 0; u < 3; ++u) {
        for (int v = 0; v < 3; ++v) {
            dest[i++] = source->m[u][v];
        }
    }
}

void matrix_scale(Matrix4fc* this, float scale, Matrix4f* dest) {
    if (this != dest) {
        matrix_copy_to(this, dest);
    }
    dest->m[0][0] = this->m[0][0] * scale;
    dest->m[1][1] = this->m[1][1] * scale;
    dest->m[2][2] = this->m[2][2] * scale;
    dest->properties &= ~(PROPERTY_IDENTITY);
}

void matrix_scale_3f(Matrix4fc* this, Vector3fc* scale, Matrix4f* dest) {
    if (this != dest) {
        matrix_copy_to(this, dest);
    }
    dest->m[0][0] = this->m[0][0] * scale->x;
    dest->m[1][1] = this->m[1][1] * scale->y;
    dest->m[2][2] = this->m[2][2] * scale->z;
    dest->properties &= ~(PROPERTY_IDENTITY | PROPERTY_ORTHONORMAL | PROPERTY_TRANSLATION);
}

void invertPerspective(Matrix4fc* this, Matrix4f* dest) {
    float m00 = this->m[0][0];
    float m11 = this->m[1][1];
    float m22 = this->m[2][2];
    float m23 = this->m[2][3];
    float m32 = this->m[3][2];
    float a = 1.0f / (m00 * m11);
    float l = -1.0f / (m23 * m32);
    *dest = (Matrix4f) {
            .m = {
                    {m11 * a, 0,       0,        0},
                    {0,       m00 * a, 0,        0},
                    {0,       0,       0,        -m23 * l},
                    {0,       0,       -m32 * l, m22 * l},
            }, .properties = 0
    };
}

void invertOrthonormal(Matrix4fc* this, Matrix4f* dest) {
    float nm30 = -(this->m[0][0] * this->m[3][0] + this->m[0][1] * this->m[3][1] + this->m[0][2] * this->m[3][2]);
    float nm31 = -(this->m[1][0] * this->m[3][0] + this->m[1][1] * this->m[3][1] + this->m[1][2] * this->m[3][2]);
    float nm32 = -(this->m[2][0] * this->m[3][0] + this->m[2][1] * this->m[3][1] + this->m[2][2] * this->m[3][2]);
    float t01 = this->m[0][1];
    float t02 = this->m[0][2];
    float t12 = this->m[1][2];
    dest->m[0][0] = (this->m[0][0]);
    dest->m[0][1] = (this->m[1][0]);
    dest->m[0][2] = (this->m[2][0]);
    dest->m[0][3] = (0.0f);
    dest->m[1][0] = (t01);
    dest->m[1][1] = (this->m[1][1]);
    dest->m[1][2] = (this->m[2][1]);
    dest->m[1][3] = (0.0f);
    dest->m[2][0] = (t02);
    dest->m[2][1] = (t12);
    dest->m[2][2] = (this->m[2][2]);
    dest->m[2][3] = (0.0f);
    dest->m[3][0] = (nm30);
    dest->m[3][1] = (nm31);
    dest->m[3][2] = (nm32);
    dest->m[3][3] = (1.0f);
    dest->properties = (PROPERTY_AFFINE | PROPERTY_ORTHONORMAL);
}

void invertAffine(Matrix4fc* this, Matrix4f* dest) {
    float m11m00 = this->m[0][0] * this->m[1][1];
    float m10m01 = this->m[0][1] * this->m[1][0];
    float m10m02 = this->m[0][2] * this->m[1][0];
    float m12m00 = this->m[0][0] * this->m[1][2];
    float m12m01 = this->m[0][1] * this->m[1][2];
    float m11m02 = this->m[0][2] * this->m[1][1];
    float det =
            (m11m00 - m10m01) * this->m[2][2] + (m10m02 - m12m00) * this->m[2][1] + (m12m01 - m11m02) * this->m[2][0];
    float s = 1.0f / det;
    float m10m22 = this->m[1][0] * this->m[2][2];
    float m10m21 = this->m[1][0] * this->m[2][1];
    float m11m22 = this->m[1][1] * this->m[2][2];
    float m11m20 = this->m[1][1] * this->m[2][0];
    float m12m21 = this->m[1][2] * this->m[2][1];
    float m12m20 = this->m[1][2] * this->m[2][0];
    float m20m02 = this->m[2][0] * this->m[0][2];
    float m20m01 = this->m[2][0] * this->m[0][1];
    float m21m02 = this->m[2][1] * this->m[0][2];
    float m21m00 = this->m[2][1] * this->m[0][0];
    float m22m01 = this->m[2][2] * this->m[0][1];
    float m22m00 = this->m[2][2] * this->m[0][0];
    float nm00 = (m11m22 - m12m21) * s;
    float nm01 = (m21m02 - m22m01) * s;
    float nm02 = (m12m01 - m11m02) * s;
    float nm10 = (m12m20 - m10m22) * s;
    float nm11 = (m22m00 - m20m02) * s;
    float nm12 = (m10m02 - m12m00) * s;
    float nm20 = (m10m21 - m11m20) * s;
    float nm21 = (m20m01 - m21m00) * s;
    float nm22 = (m11m00 - m10m01) * s;
    float nm30 = (m10m22 * this->m[3][1] - m10m21 * this->m[3][2] + m11m20 * this->m[3][2] - m11m22 * this->m[3][0] +
                  m12m21 * this->m[3][0] - m12m20 * this->m[3][1]) * s;
    float nm31 = (m20m02 * this->m[3][1] - m20m01 * this->m[3][2] + m21m00 * this->m[3][2] - m21m02 * this->m[3][0] +
                  m22m01 * this->m[3][0] - m22m00 * this->m[3][1]) * s;
    float nm32 = (m11m02 * this->m[3][0] - m12m01 * this->m[3][0] + m12m00 * this->m[3][1] - m10m02 * this->m[3][1] +
                  m10m01 * this->m[3][2] - m11m00 * this->m[3][2]) * s;
    dest->m[0][0] = (nm00);
    dest->m[0][1] = (nm01);
    dest->m[0][2] = (nm02);
    dest->m[0][3] = (0.0f);
    dest->m[1][0] = (nm10);
    dest->m[1][1] = (nm11);
    dest->m[1][2] = (nm12);
    dest->m[1][3] = (0.0f);
    dest->m[2][0] = (nm20);
    dest->m[2][1] = (nm21);
    dest->m[2][2] = (nm22);
    dest->m[2][3] = (0.0f);
    dest->m[3][0] = (nm30);
    dest->m[3][1] = (nm31);
    dest->m[3][2] = (nm32);
    dest->m[3][3] = (1.0f);
    dest->properties = (PROPERTY_AFFINE);
}


void invertGeneric(Matrix4fc* this, Matrix4f* dest) {
    float a = this->m[0][0] * this->m[1][1] - this->m[0][1] * this->m[1][0];
    float b = this->m[0][0] * this->m[1][2] - this->m[0][2] * this->m[1][0];
    float c = this->m[0][0] * this->m[1][3] - this->m[0][3] * this->m[1][0];
    float d = this->m[0][1] * this->m[1][2] - this->m[0][2] * this->m[1][1];
    float e = this->m[0][1] * this->m[1][3] - this->m[0][3] * this->m[1][1];
    float f = this->m[0][2] * this->m[1][3] - this->m[0][3] * this->m[1][2];
    float g = this->m[2][0] * this->m[3][1] - this->m[2][1] * this->m[3][0];
    float h = this->m[2][0] * this->m[3][2] - this->m[2][2] * this->m[3][0];
    float i = this->m[2][0] * this->m[3][3] - this->m[2][3] * this->m[3][0];
    float j = this->m[2][1] * this->m[3][2] - this->m[2][2] * this->m[3][1];
    float k = this->m[2][1] * this->m[3][3] - this->m[2][3] * this->m[3][1];
    float l = this->m[2][2] * this->m[3][3] - this->m[2][3] * this->m[3][2];
    float det = a * l - b * k + c * j + d * i - e * h + f * g;
    det = 1.0f / det;
    float nm00 = (this->m[1][1] * l - this->m[1][2] * k + this->m[1][3] * j) * det;
    float nm01 = (-this->m[0][1] * l + this->m[0][2] * k - this->m[0][3] * j) * det;
    float nm02 = (this->m[3][1] * f - this->m[3][2] * e + this->m[3][3] * d) * det;
    float nm03 = (-this->m[2][1] * f + this->m[2][2] * e - this->m[2][3] * d) * det;
    float nm10 = (-this->m[1][0] * l + this->m[1][2] * i - this->m[1][3] * h) * det;
    float nm11 = (this->m[0][0] * l - this->m[0][2] * i + this->m[0][3] * h) * det;
    float nm12 = (-this->m[3][0] * f + this->m[3][2] * c - this->m[3][3] * b) * det;
    float nm13 = (this->m[2][0] * f - this->m[2][2] * c + this->m[2][3] * b) * det;
    float nm20 = (this->m[1][0] * k - this->m[1][1] * i + this->m[1][3] * g) * det;
    float nm21 = (-this->m[0][0] * k + this->m[0][1] * i - this->m[0][3] * g) * det;
    float nm22 = (this->m[3][0] * e - this->m[3][1] * c + this->m[3][3] * a) * det;
    float nm23 = (-this->m[2][0] * e + this->m[2][1] * c - this->m[2][3] * a) * det;
    float nm30 = (-this->m[1][0] * j + this->m[1][1] * h - this->m[1][2] * g) * det;
    float nm31 = (this->m[0][0] * j - this->m[0][1] * h + this->m[0][2] * g) * det;
    float nm32 = (-this->m[3][0] * d + this->m[3][1] * b - this->m[3][2] * a) * det;
    float nm33 = (this->m[2][0] * d - this->m[2][1] * b + this->m[2][2] * a) * det;
    dest->m[0][0] = (nm00);
    dest->m[0][1] = (nm01);
    dest->m[0][2] = (nm02);
    dest->m[0][3] = (nm03);
    dest->m[1][0] = (nm10);
    dest->m[1][1] = (nm11);
    dest->m[1][2] = (nm12);
    dest->m[1][3] = (nm13);
    dest->m[2][0] = (nm20);
    dest->m[2][1] = (nm21);
    dest->m[2][2] = (nm22);
    dest->m[2][3] = (nm23);
    dest->m[3][0] = (nm30);
    dest->m[3][1] = (nm31);
    dest->m[3][2] = (nm32);
    dest->m[3][3] = (nm33);
    dest->properties = 0;
}

void matrix_invert(Matrix4fc* this, Matrix4f* dest) {
    if ((this->properties & PROPERTY_IDENTITY) != 0) {
        matrix_set_identity(dest);
    } else if ((this->properties & PROPERTY_ORTHONORMAL) != 0) {
        invertOrthonormal(this, dest);
    } else if ((this->properties & PROPERTY_AFFINE) != 0) {
        invertAffine(this, dest);
    } else if ((this->properties & PROPERTY_PERSPECTIVE) != 0) {
        invertPerspective(this, dest);
    } else {
        invertGeneric(this, dest);
    }
}