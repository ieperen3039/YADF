//
// Created by s152717 on 28-12-2019.
//

#ifndef YADF_BOUNDINGBOX_H
#define YADF_BOUNDINGBOX_H

#include <stdlib.h>
#include <limits.h>

#include "Structs.h"
#include "../Tools.h"
#include "Vector3f.h"
#include "Matrix4f.h"

typedef struct {
    int xMin;
    int yMin;
    int zMin;
    int xMax;
    int yMax;
    int zMax;
} BoundingBox;

static const BoundingBox BOUNDING_BOX_EMPTY = {INT_MAX, INT_MAX, INT_MAX, INT_MIN, INT_MIN, INT_MIN};

static void bounding_box_add(BoundingBox* box, Vector3i point) {
    box->xMin = min_i(point.x, box->xMin);
    box->yMin = min_i(point.y, box->yMin);
    box->zMin = min_i(point.z, box->zMin);
    box->xMax = max_i(point.x, box->xMax);
    box->yMax = max_i(point.y, box->yMax);
    box->zMax = max_i(point.z, box->zMax);
}

static void bounding_box_get_view_box(BoundingBox* box, Matrix4fc* projection) {
    Matrix4f inv_proj;
    matrix_invert(projection, &inv_proj);

    for (int x = -1; x <= 1; x += 2) {
        for (int y = -1; y <= 1; y += 2) {
            for (int z = -1; z <= 1; z += 2) {
                Vector3f vec = {x, y, z};
                vector_mul_position(&vec, &inv_proj);

                bounding_box_add(box, (Vector3i) {(int) vec.x, (int) vec.y, (int) vec.z});
            }
        }
    }
}

#endif //YADF_BOUNDINGBOX_H
