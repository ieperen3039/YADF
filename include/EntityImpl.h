//
// Created by s152717 on 5-1-2020.
//

#ifndef YADF_ENTITYIMPL_H
#define YADF_ENTITYIMPL_H

#include <Materials.h>

struct EntityImplNaturalWall {
    enum Material material;
};

ENUM( Vegetation,
    GRASS
)

struct EntityImplGrass {
    enum Vegetation type;
};

#endif //YADF_ENTITYIMPL_H
