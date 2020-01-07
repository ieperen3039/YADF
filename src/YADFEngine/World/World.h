//
// Created by ieperen3039 on 27-11-19.
//

#ifndef YADF2_WORLD_H
#define YADF2_WORLD_H

#include "WorldAPI.h"

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
int world_initialize_area(World* world, const BoundingBox area, const WorldTile initial_tile);

/**
 * returns an iterator over all entities requiring an update
 * */
PURE List* world_get_entities_to_update(World* world);

void world_tile_init(WorldTile* base_tile, char flags);

#endif //YADF2_WORLD_H