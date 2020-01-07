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

#endif //YADF_BOUNDINGBOX_H
