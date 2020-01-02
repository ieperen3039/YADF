//
// Created by ieperen3039 on 27-11-19.
//

#ifndef YADF2_WORLD_H
#define YADF2_WORLD_H

#include <stdlib.h>

#include "Structs.h"
#include "../DataStructures/List.h"
#include "../DataStructures/StaticMonoAllocator.h"
#include "Materials.h"
#include "../DataStructures/BoundingBox.h"

#define CHUNK_COORD_BITS 4
#define CHUNK_LENGTH (1 << CHUNK_COORD_BITS)

#define WORLD_DEFAULT_ENTITY_LIST_CAPACITY 256

#define TILE_FLAG_OPAQUE (1 << 0)
#define TILE_FLAG_ABOVE_GAP (1 << 1)
#define TILE_FLAG_VISIBLE (1 << 2)
#define TILE_FLAG_DISCOVERED (1 << 3)
//#define TILE_FLAG_ (1 << 0)

typedef struct {
    // items on this tile
    List entity_ptrs; // type = Entity*
    // various TILE_FLAGs
    char flags;
} WorldTile;

typedef struct _World World;
typedef struct _WorldQuadrant WorldQuadrant;
typedef struct _WorldChunk WorldChunk;

/// accepts a tile and the coordinate of that tile
typedef void(* TileFunction)(WorldTile* tile, int x, int y, int z);

/**
 * Creates a new world, without any chunks initialized, of size `world_min_size`, around (0,0,0)
 * @param world_min_size the minimum size of this world in one dimension in nr of tiles
 * @return the new world, without any chunks initialized
 */
World* world_new(int world_min_size);

/** frees all memory associated with world */
void world_free(World* world);

/**
 * given a nearby chunk, returns the tile on the given position.
 * This is likely to be faster than {@link world_get_tile} iff pos is indeed close to the given chunk
 * @param chunk a chunk, possibly containing the given pos, otherwise close to the given chunk
 * @param pos the coordinate of the requested tile
 * @return the tile on the given coordinate, or NULL if this one is in an unloaded chunk.
 */
PURE WorldTile* world_get_tile_from_chunk(WorldChunk* chunk, Vector3i coord);

/**
 * executes the given action for every tile at the given layer in the given chunk
 */
void world_exec_layer(WorldChunk* chunk, int layer, TileFunction action);

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

typedef struct {
    WorldQuadrant* targetQuad;
    BoundingBox bounds;
    Vector3i focus;
} WorldChunkIterator;

/**
 * creates an iterator of at least the chunks that include the coordinates of the bounding box
 */
WorldChunkIterator world_get_chunk_iterator(World* world, BoundingBox box);

typedef struct {
    /// the chunk itself, or NULL if the chunk is uninitialized
    WorldChunk* elt;
    /// world zero-pos of this tile.
    Vector3i coord;
} WorldChunkData;

/** returns the next chunk. Returns NULL if the chunk is not initialized */
WorldChunkData world_chunk_iterator_next(WorldChunkIterator* itr);

/** returns true iff there are more chunks in the iterator */
bool world_chunk_iterator_has_next(WorldChunkIterator* itr);

typedef struct {
    Vector3i chunk_pos;
    WorldTile* data;
    int index;
} WorldTileIterator;

/** creates an iterator to query all tiles in a chunk, with their respective coordinate */
WorldTileIterator chunk_get_tile_iterator(WorldChunk* chunk);

typedef struct {
    /// the tile itself
    WorldTile* elt;
    /// world position of this tile
    Vector3i coord;
} WorldTileData;

/// returns the next tile-coordinate pair. The coordinate is in world-space
WorldTileData chunk_tile_iterator_next(WorldTileIterator* itr);

/// returns true iff there are more tiles in the given iterator
bool chunk_tile_iterator_has_next(WorldTileIterator* itr);

Vector3ic* chunk_get_position(WorldChunk* chunk);

void world_tile_init(WorldTile* base_tile, char flags);

#endif //YADF2_WORLD_H