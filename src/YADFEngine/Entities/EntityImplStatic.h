//
// Created by s152717 on 3-1-2020.
//
#ifndef YADF_ENTITYIMPLSTATIC_H
#define YADF_ENTITYIMPLSTATIC_H

#include "Materials.h"

typedef struct _Entity Entity;
typedef struct _Shader Shader;

struct EntityImplGrass {
};

void entity_impl_grass_update(Entity* this);

struct EntityImplNaturalWall {
};

void entity_impl_natural_wall_update(Entity* this);

//struct EntityImpl# {};
//void entity_impl_#_update();
//void entity_impl_#_render();

#endif //YADF_ENTITYIMPLSTATIC_H
