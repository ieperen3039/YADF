//
// Created by s152717 on 27-12-2019.
//

#ifndef YADF_ENTITYINSTANCE_H
#define YADF_ENTITYINSTANCE_H

#include "Entity.h"
#include "Structs.h"
#include "../World/World.h"

//#define ENTITY_FLAG_LIVING (1 << 0)
//#define ENTITY_FLAG_ (1 << 1)
//#define ENTITY_FLAG_ (1 << 2)
//#define ENTITY_FLAG_ (1 << 3)
//#define ENTITY_FLAG_ (1 << 4)

static inline void entity_update(Entity* ety) {
    ety->class->update(ety);
}

/**
 * The metadata of this entity is set to a copy of the pointed data.
 * If initial_data is NULL, then the entity_data field will be NULL as well.
 * Do not try to reallocate the entity_data field.
 * @return the new entity
 */
Entity* entity_new(enum EntityClass type, const void* initial_data);

/**
 * Adds the given entity to the tile
 * @param tile the tile to add this entity's pointer to
 * @param the entity whomst pointer must be added to this list
 */
void world_tile_add_entity(WorldTileData tile, Entity* entity, WorldChunk* chunk);

#endif //YADF_ENTITYINSTANCE_H
