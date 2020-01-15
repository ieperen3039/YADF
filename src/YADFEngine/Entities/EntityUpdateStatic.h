//
// Created by s152717 on 3-1-2020.
//
#ifndef YADF_ENTITYUPDATESTATIC_H
#define YADF_ENTITYUPDATESTATIC_H

#include <Entity.h>

void entity_impl_empty_update(Entity* this, UpdateCycle time);

void entity_impl_grass_apply(Entity* this, Vector3ic* coordinate, WorldTile* tile);

void entity_impl_natural_wall_apply(Entity* this, Vector3ic* coordinate, WorldTile* tile);

//struct EntityImpl# {};
//void entity_impl_#_update();
//void entity_impl_#_render();

#endif //YADF_ENTITYUPDATESTATIC_H
