//
// Created by s152717 on 5-1-2020.
//

#ifndef YADF_WORLDAPI_H
#define YADF_WORLDAPI_H

#include "APIDefine.h"
#include "Structs.h"
#include "../src/YADFEngine/DataStructures/List.h"
#include "../src/YADFEngine/DataStructures/BoundingBox.h"

#define TILE_FLAG_BLOCKING (1 << 0)
#define TILE_FLAG_ABOVE_GAP (1 << 1)
#define TILE_FLAG_VISIBLE (1 << 2)
#define TILE_FLAG_DISCOVERED (1 << 3)
//#define TILE_FLAG_ (1 << 0)

typedef struct _FluidFlow FluidFlow;

typedef struct {
    int index; // index of this tile in parent chunk
    // items on this tile
    List entity_ptrs; // type = Entity*
    // various TILE_FLAGs
    int flags;
} WorldTile;

typedef struct _World World;
typedef struct _WorldChunk WorldChunk;
typedef struct _WorldQuadrant WorldQuadrant;

/**
 * returns the tile on the given coordinate
 * @param world a generated world
 * @param coord the coordinate to fetch
 * @return the tile on the given coordinate, or NULL if the coordinate is in an unloaded chunk
 */
YADF_API WorldTile* world_get_tile(World* world, Vector3ic* coord);

YADF_API WorldChunk* world_get_chunk(World* world, Vector3ic* coord);

YADF_API PURE Vector3i world_get_tile_coord(WorldTile* tile, WorldChunk* parent);

/**
 * given a nearby chunk, returns the tile on the given position.
 * This is likely to be faster than world_get_tile if pos is indeed close to the given chunk
 * @param chunk a chunk, possibly containing the given pos, otherwise close to the given chunk
 * @param pos the coordinate of the requested tile
 * @return the tile on the given coordinate, or NULL if this one is in an unloaded chunk.
 */
YADF_API WorldTile* world_get_tile_from_chunk(WorldChunk* chunk, Vector3ic* coord);

YADF_API WorldChunk* world_get_chunk_from_chunk(WorldChunk* chunk, Vector3ic* coord);

typedef struct {
    WorldQuadrant* targetQuad;
    BoundingBox bounds;
    Vector3i zero;
    Vector3i focus;
} WorldChunkIterator;

/**
 * creates an iterator of at least the chunks that include the coordinates of the bounding box
 */
YADF_API WorldChunkIterator world_get_chunk_iterator(World* world, BoundingBox box);

typedef struct {
    /// the chunk itself, or NULL if the chunk is uninitialized
    WorldChunk* elt;
    /// world zero-pos of this tile.
    Vector3i coord;
} WorldChunkData;

/** returns the next chunk. Returns NULL if the chunk is not initialized */
YADF_API WorldChunkData world_chunk_iterator_next(WorldChunkIterator* itr);

/** returns true iff there are more chunks in the iterator */
YADF_API bool world_chunk_iterator_has_next(WorldChunkIterator* itr);

typedef struct {
    Vector3i chunk_pos;
    Vector3i tile_in_chunk_pos;
    WorldChunk* chunk;
} WorldTileIterator;

/** creates an iterator to query all tiles in a chunk, with their respective coordinate */
YADF_API WorldTileIterator chunk_get_tile_iterator(WorldChunk* chunk);

typedef struct {
    /// the tile itself
    WorldTile* elt;
    /// world position of this tile
    Vector3i coord;
} WorldTileData;

/// returns the next tile-coordinate pair. The coordinate is in world-space
YADF_API WorldTileData chunk_tile_iterator_next(WorldTileIterator* itr);

/// returns true iff there are more tiles in the given iterator
YADF_API bool chunk_tile_iterator_has_next(WorldTileIterator* itr);

typedef struct {
    const World* world;
    WorldQuadrant* quad;
    Vector3i current;
    int x_sub_y_min;
    int x_sub_y_max;
    int x_add_y_min;
    int x_add_y_max;
    const int z_min;
    const bool xp, yp; // iteration direction per dimension is positive? (z is always false)
} WorldDirectionalIterator;

/**
 *
 * an iterator specifically to iterate through an architectural perspective
 * @param world the world to query the tiles from
 * @param focus the middle of the are to iterate
 * @param width number of tiles returned in width
 * @param height number of tiles returned in depth
 * @param x_pos if true, iterate x in positive direction. if false, iterate x in negative direction
 * @param y_pos if true, iterate y in positive direction. if false, iterate y in negative direction
 * @param z_min the minimum z value to return
 * @return
 */
YADF_API WorldDirectionalIterator
world_directional_iterator(const World* world, Vector3fc* focus, float width, float height, bool x_pos, bool y_pos,
                           int z_min);

YADF_API WorldTileData world_directional_iterator_next(WorldDirectionalIterator* itr);

YADF_API bool world_directional_iterator_has_next(WorldDirectionalIterator* itr);

YADF_API PURE Vector3ic* chunk_get_position(WorldChunk* chunk);

YADF_API FluidFlow* world_get_fluid(World* world, Vector3ic* coord);

YADF_API FluidFlow* tile_get_fluid(WorldTile* tile, WorldChunk* chunk);

#endif //YADF_WORLDAPI_H
