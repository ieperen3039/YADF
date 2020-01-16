//
// Created by s152717 on 3-1-2020.
// some update functions for some entities
//

#ifndef YADF_ENTITYUPDATESTATIC_H
#define YADF_ENTITYUPDATESTATIC_H

#include <Entity.h>
#include <WorldAPI.h>

/**
 * Pre-update function that does nothing
 */
void entity_impl_empty_update(Entity* this, UpdateCycle time);

/**
 * post-update function that does nothing
 */
void entity_impl_no_apply(Entity* this, Vector3ic* coordinate, WorldTile* tile);

#endif //YADF_ENTITYUPDATESTATIC_H
