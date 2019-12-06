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
    float m00 = first->m00 * second->m00 + first->m10 * second->m01 + first->m20 * second->m02;
    float m01 = first->m01 * second->m00 + first->m11 * second->m01 + first->m21 * second->m02;
    float m02 = first->m02 * second->m00 + first->m12 * second->m01 + first->m22 * second->m02;
    float m03 = first->m03;
    float m10 = first->m00 * second->m10 + first->m10 * second->m11 + first->m20 * second->m12;
    float m11 = first->m01 * second->m10 + first->m11 * second->m11 + first->m21 * second->m12;
    float m12 = first->m02 * second->m10 + first->m12 * second->m11 + first->m22 * second->m12;
    float m13 = first->m13;
    float m20 = first->m00 * second->m20 + first->m10 * second->m21 + first->m20 * second->m22;
    float m21 = first->m01 * second->m20 + first->m11 * second->m21 + first->m21 * second->m22;
    float m22 = first->m02 * second->m20 + first->m12 * second->m21 + first->m22 * second->m22;
    float m23 = first->m23;
    float m30 = first->m00 * second->m30 + first->m10 * second->m31 + first->m20 * second->m32 + first->m30;
    float m31 = first->m01 * second->m30 + first->m11 * second->m31 + first->m21 * second->m32 + first->m31;
    float m32 = first->m02 * second->m30 + first->m12 * second->m31 + first->m22 * second->m32 + first->m32;
    // split for the case (dest == first || dest == second)
    dest->m00 = m00;
    dest->m01 = m01;
    dest->m02 = m02;
    dest->m03 = m03;
    dest->m10 = m10;
    dest->m11 = m11;
    dest->m12 = m12;
    dest->m13 = m13;
    dest->m20 = m20;
    dest->m21 = m21;
    dest->m22 = m22;
    dest->m23 = m23;
    dest->m30 = m30;
    dest->m31 = m31;
    dest->m32 = m32;

    dest->properties = (PROPERTY_AFFINE | (first->properties & second->properties & PROPERTY_ORTHONORMAL));
}

void mulPerspectiveAffine(Matrix4f* first, Matrix4f* second, Matrix4f* dest) {
    float m00 = first->m00 * second->m00;
    float m01 = first->m11 * second->m01;
    float m02 = first->m22 * second->m02;
    float m03 = first->m23 * second->m02;
    float m10 = first->m00 * second->m10;
    float m11 = first->m11 * second->m11;
    float m12 = first->m22 * second->m12;
    float m13 = first->m23 * second->m12;
    float m20 = first->m00 * second->m20;
    float m21 = first->m11 * second->m21;
    float m22 = first->m22 * second->m22;
    float m23 = first->m23 * second->m22;
    float m30 = first->m00 * second->m30;
    float m31 = first->m11 * second->m31;
    float m32 = first->m22 * second->m32 + first->m32;
    float m33 = first->m23 * second->m32;
    dest->m00 = m00;
    dest->m01 = m01;
    dest->m02 = m02;
    dest->m03 = m03;
    dest->m10 = m10;
    dest->m11 = m11;
    dest->m12 = m12;
    dest->m13 = m13;
    dest->m20 = m20;
    dest->m21 = m21;
    dest->m22 = m22;
    dest->m23 = m23;
    dest->m30 = m30;
    dest->m31 = m31;
    dest->m32 = m32;
    dest->m33 = m33;

    dest->properties = 0;
}

void mulAffineR(Matrix4f* first, Matrix4f* second, Matrix4f* dest) {
    float m00 = first->m00 * second->m00 + first->m10 * second->m01 + first->m20 * second->m02;
    float m01 = first->m01 * second->m00 + first->m11 * second->m01 + first->m21 * second->m02;
    float m02 = first->m02 * second->m00 + first->m12 * second->m01 + first->m22 * second->m02;
    float m03 = first->m03 * second->m00 + first->m13 * second->m01 + first->m23 * second->m02;
    float m10 = first->m00 * second->m10 + first->m10 * second->m11 + first->m20 * second->m12;
    float m11 = first->m01 * second->m10 + first->m11 * second->m11 + first->m21 * second->m12;
    float m12 = first->m02 * second->m10 + first->m12 * second->m11 + first->m22 * second->m12;
    float m13 = first->m03 * second->m10 + first->m13 * second->m11 + first->m23 * second->m12;
    float m20 = first->m00 * second->m20 + first->m10 * second->m21 + first->m20 * second->m22;
    float m21 = first->m01 * second->m20 + first->m11 * second->m21 + first->m21 * second->m22;
    float m22 = first->m02 * second->m20 + first->m12 * second->m21 + first->m22 * second->m22;
    float m23 = first->m03 * second->m20 + first->m13 * second->m21 + first->m23 * second->m22;
    float m30 = first->m00 * second->m30 + first->m10 * second->m31 + first->m20 * second->m32 + first->m30;
    float m31 = first->m01 * second->m30 + first->m11 * second->m31 + first->m21 * second->m32 + first->m31;
    float m32 = first->m02 * second->m30 + first->m12 * second->m31 + first->m22 * second->m32 + first->m32;
    float m33 = first->m03 * second->m30 + first->m13 * second->m31 + first->m23 * second->m32 + first->m33;

    dest->m00 = m00;
    dest->m01 = m01;
    dest->m02 = m02;
    dest->m03 = m03;
    dest->m10 = m10;
    dest->m11 = m11;
    dest->m12 = m12;
    dest->m13 = m13;
    dest->m20 = m20;
    dest->m21 = m21;
    dest->m22 = m22;
    dest->m23 = m23;
    dest->m30 = m30;
    dest->m31 = m31;
    dest->m32 = m32;
    dest->m33 = m33;
    dest->properties = (first->properties &
                        ~(PROPERTY_IDENTITY | PROPERTY_PERSPECTIVE | PROPERTY_TRANSLATION | PROPERTY_ORTHONORMAL));
}

void mulGeneric(Matrix4f* first, Matrix4f* second, Matrix4f* dest) {
    float m00 =
            first->m00 * second->m00 + first->m10 * second->m01 + first->m20 * second->m02 + first->m30 * second->m03;
    float m01 =
            first->m01 * second->m00 + first->m11 * second->m01 + first->m21 * second->m02 + first->m31 * second->m03;
    float m02 =
            first->m02 * second->m00 + first->m12 * second->m01 + first->m22 * second->m02 + first->m32 * second->m03;
    float m03 =
            first->m03 * second->m00 + first->m13 * second->m01 + first->m23 * second->m02 + first->m33 * second->m03;
    float m10 =
            first->m00 * second->m10 + first->m10 * second->m11 + first->m20 * second->m12 + first->m30 * second->m13;
    float m11 =
            first->m01 * second->m10 + first->m11 * second->m11 + first->m21 * second->m12 + first->m31 * second->m13;
    float m12 =
            first->m02 * second->m10 + first->m12 * second->m11 + first->m22 * second->m12 + first->m32 * second->m13;
    float m13 =
            first->m03 * second->m10 + first->m13 * second->m11 + first->m23 * second->m12 + first->m33 * second->m13;
    float m20 =
            first->m00 * second->m20 + first->m10 * second->m21 + first->m20 * second->m22 + first->m30 * second->m23;
    float m21 =
            first->m01 * second->m20 + first->m11 * second->m21 + first->m21 * second->m22 + first->m31 * second->m23;
    float m22 =
            first->m02 * second->m20 + first->m12 * second->m21 + first->m22 * second->m22 + first->m32 * second->m23;
    float m23 =
            first->m03 * second->m20 + first->m13 * second->m21 + first->m23 * second->m22 + first->m33 * second->m23;
    float m30 =
            first->m00 * second->m30 + first->m10 * second->m31 + first->m20 * second->m32 + first->m30 * second->m33;
    float m31 =
            first->m01 * second->m30 + first->m11 * second->m31 + first->m21 * second->m32 + first->m31 * second->m33;
    float m32 =
            first->m02 * second->m30 + first->m12 * second->m31 + first->m22 * second->m32 + first->m32 * second->m33;
    float m33 =
            first->m03 * second->m30 + first->m13 * second->m31 + first->m23 * second->m32 + first->m33 * second->m33;

    dest->m00 = m00;
    dest->m01 = m01;
    dest->m02 = m02;
    dest->m03 = m03;
    dest->m10 = m10;
    dest->m11 = m11;
    dest->m12 = m12;
    dest->m13 = m13;
    dest->m20 = m20;
    dest->m21 = m21;
    dest->m22 = m22;
    dest->m23 = m23;
    dest->m30 = m30;
    dest->m31 = m31;
    dest->m32 = m32;
    dest->m33 = m33;

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

void matrix_copy_to(Matrix4f* source, Matrix4f* dest) {
    if (source == dest) return;
    memcpy(dest, source, sizeof(Matrix4f));
}

Vector3f* matrix_project(Matrix4f* this, float x, float y, float z, const int* viewport, Vector3f* winCoordsDest) {
    float invW = 1.0f / (this->m03 * x + this->m13 * y + this->m23 * z + this->m33);
    float nx = (this->m00 * x + this->m10 * y + this->m20 * z + this->m30) * invW;
    float ny = (this->m01 * x + this->m11 * y + this->m21 * z + this->m31) * invW;
    float nz = (this->m02 * x + this->m12 * y + this->m22 * z + this->m32) * invW;
    winCoordsDest->x = (float) ((nx * 0.5 + 0.5) * viewport[2] + viewport[0]);
    winCoordsDest->y = (float) ((ny * 0.5 + 0.5) * viewport[3] + viewport[1]);
    winCoordsDest->z = (1.0f + nz) * 0.5f;
    return winCoordsDest;
}

Matrix4f matrix_get_ortho_projection(float width, float height, float zNear, float zFar) {
    Matrix4f this = matrix_get_identity();
    this.m00 = (2.0f / width);
    this.m11 = (2.0f / height);
    this.m22 = (2.0f / (zNear - zFar));
    this.m32 = ((zFar + zNear) / (zNear - zFar));
    this.properties = PROPERTY_AFFINE;
    return this;
}

Matrix4f matrix_get_ortho_projection_unsigned(float width, float height, float zNear, float zFar) {
    Matrix4f this = matrix_get_identity();
    this.m00 = (2.0f / width);
    this.m11 = (2.0f / height);
    this.m22 = (1.0f / (zNear - zFar));
    this.m32 = (zNear / (zNear - zFar));
    this.properties = PROPERTY_AFFINE;
    return this;
}

void normal_generic(const Matrix4f* this, Matrix4f* dest) {
    float m00m11 = this->m00 * this->m11;
    float m01m10 = this->m01 * this->m10;
    float m02m10 = this->m02 * this->m10;
    float m00m12 = this->m00 * this->m12;
    float m01m12 = this->m01 * this->m12;
    float m02m11 = this->m02 * this->m11;
    float det = (m00m11 - m01m10) * this->m22 + (m02m10 - m00m12) * this->m21 + (m01m12 - m02m11) * this->m20;
    float s = 1.0f / det;
    /* Invert and transpose in one go */
    float nm00 = (this->m11 * this->m22 - this->m21 * this->m12) * s;
    float nm01 = (this->m20 * this->m12 - this->m10 * this->m22) * s;
    float nm02 = (this->m10 * this->m21 - this->m20 * this->m11) * s;
    float nm10 = (this->m21 * this->m02 - this->m01 * this->m22) * s;
    float nm11 = (this->m00 * this->m22 - this->m20 * this->m02) * s;
    float nm12 = (this->m20 * this->m01 - this->m00 * this->m21) * s;
    float nm20 = (m01m12 - m02m11) * s;
    float nm21 = (m02m10 - m00m12) * s;
    float nm22 = (m00m11 - m01m10) * s;

    dest->m00 = (nm00);
    dest->m01 = (nm01);
    dest->m02 = (nm02);
    dest->m03 = (0.0f);
    dest->m10 = (nm10);
    dest->m11 = (nm11);
    dest->m12 = (nm12);
    dest->m13 = (0.0f);
    dest->m20 = (nm20);
    dest->m21 = (nm21);
    dest->m22 = (nm22);
    dest->m23 = (0.0f);
    dest->m30 = (0.0f);
    dest->m31 = (0.0f);
    dest->m32 = (0.0f);
    dest->m33 = (1.0f);
    dest->properties = (this->properties | PROPERTY_AFFINE) & ~(PROPERTY_TRANSLATION | PROPERTY_PERSPECTIVE);
}

void matrix_normal(Matrix4f* this, Matrix4f* dest) {
    if ((this->properties & PROPERTY_IDENTITY) != 0) {
        matrix_set_identity(dest);

    } else if ((this->properties & PROPERTY_ORTHONORMAL) != 0) {
        matrix_copy_to(this, dest);

    } else {
        normal_generic(this, dest);
    }
}

bool matrix_equals(Matrix4f* one, Matrix4f* two) {
    return
            one->m00 == two->m00 &&
            one->m01 == two->m01 &&
            one->m02 == two->m02 &&
            one->m03 == two->m03 &&
            one->m10 == two->m10 &&
            one->m11 == two->m11 &&
            one->m12 == two->m12 &&
            one->m13 == two->m13 &&
            one->m20 == two->m20 &&
            one->m21 == two->m21 &&
            one->m22 == two->m22 &&
            one->m23 == two->m23 &&
            one->m30 == two->m30 &&
            one->m31 == two->m31 &&
            one->m32 == two->m32 &&
            one->m33 == two->m33;
}
