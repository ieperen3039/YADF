//
// Created by ieperen3039 on 27-11-19.
//

#ifndef YADF2_WORLD_H
#define YADF2_WORLD_H
#include <stdlib.h>

#include "DataStructures/List.h"
#include "Structs.h"
#include "DataStructures/StaticMonoAllocator.h"
#include "Materials.h"

#define CHUNK_COORD_BITS 4
#define CHUNK_LENGTH (1 << CHUNK_COORD_BITS)

typedef struct { // initialized as zero
    List item_ptrs;
    enum Material material;
    bool isOpaque;
} WorldTile;

typedef struct {
    WorldTile tiles[CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_LENGTH];
} WorldChunk;

typedef struct _World World;

// accepts a tile and the coordinate of that tile
typedef void(*TileFunction)(WorldTile, int x, int y, int z);

/**
 * Creates a new world, uninitialized, of size `world_min_size` around (0,0,0)
 * @param world_min_size the minimum size of this world in one dimension in nr of tiles
 * @return the new world, without any chunks initialized
 */
World* world_new(int world_min_size);

/**
 * @param coord world coordinate
 * @return a specific tile
 */
WorldTile* world_get_tile(World* world, Vector3i coord);

/**
 * executes the given action for every tile at the given layer in the given chunk
 */
void world_exec_layer(WorldChunk* chunk, int layer, TileFunction action);

#endif //YADF2_WORLD_H
