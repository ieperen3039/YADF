//
// Created by ieperen3039 on 27-11-19.
//

#ifndef YADF2_WORLD_H
#define YADF2_WORLD_H

#include "WorldAPI.h"
typedef struct _Entity Entity;

#define WORLD_DEFAULT_ENTITY_LIST_CAPACITY 256

#define CHUNK_COORD_BITS 4
#define CHUNK_LENGTH (1 << CHUNK_COORD_BITS)

/**
 * Creates a new world, without any chunks initialized, of size `world_min_size`, around (0,0,0)
 * @param world_min_size the minimum size of this world in one dimension in nr of tiles
 * @return the new world, without any chunks initialized
 */
World* world_new(int world_min_size);

/** frees all memory associated with world */
void world_free(World* world);

/**
 * Initialize an area at least covering the given area with tiles of initial_tile. Already initialized chunks are not initialized again.
 * @param world reference to the world
 * @param area minimum area to initialize
 * @return the number of newly initialized chunks
 */
int world_initialize_area(World* world, const BoundingBox area, int initial_flags);

/**
 * returns a list of type Entity* of all entities requiring an update
 * */
PURE List* world_get_entities_to_update(World* world);

typedef struct {
  FluidFlow* element;
  WorldChunk* chunk;
  Vector3i coord;
} FluidUpdateData;

/// returns a list of type FluidUpdateData
PURE List* world_get_fluids_to_update(World* world);

/**
 * Adds the given entity to the tile
 * @param tile the tile to add this entity's pointer to
 * @param the entity whomst pointer must be added to this list
 */
void world_tile_add_entity(Entity* entity, WorldChunk* chunk, WorldTile* tile, Vector3ic* coord);

void world_tile_remove_entity(WorldTile* tile, Entity* entity);

#endif //YADF2_WORLD_H