//
// Created by s152717 on 27-12-2019.
// Functions for modifying entities
//

#ifndef YADF_ENTITYWRITE_H
#define YADF_ENTITYWRITE_H

#include "Entity.h"
#include "Structs.h"
#include "../World/World.h"

//#define ENTITY_FLAG_LIVING (1 << 0)
//#define ENTITY_FLAG_ (1 << 1)
//#define ENTITY_FLAG_ (1 << 2)
//#define ENTITY_FLAG_ (1 << 3)
//#define ENTITY_FLAG_ (1 << 4)

/**
 * The metadata of this entity is set to a copy of the pointed data.
 * If initial_data is NULL, then the entity_data field will be NULL as well.
 * Do not try to reallocate the entity_data field.
 * @return the new entity
 */
Entity* entity_new(enum EntityClass type, const void* initial_data);

void entity_free(Entity* ety);

#endif //YADF_ENTITYWRITE_H
