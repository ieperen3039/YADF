//
// Created by s152717 on 5-1-2020.
//

#ifndef YADF_WORLDAPI_H
#define YADF_WORLDAPI_H

#include "APIDefine.h"
#include "Structs.h"
#include "../src/YADFEngine/DataStructures/List.h"
#include "../src/YADFEngine/DataStructures/BoundingBox.h"

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
typedef struct _WorldChunk WorldChunk;
typedef struct _WorldQuadrant WorldQuadrant;

/**
 * given a nearby chunk, returns the tile on the given position.
 * This is likely to be faster than world_get_tile if pos is indeed close to the given chunk
 * @param chunk a chunk, possibly containing the given pos, otherwise close to the given chunk
 * @param pos the coordinate of the requested tile
 * @return the tile on the given coordinate, or NULL if this one is in an unloaded chunk.
 */
YADF_API PURE WorldTile* world_get_tile_from_chunk(WorldChunk* chunk, Vector3i coord);

typedef struct {

} WorldDirectionalIterator;


//TODO YADF_API WorldDirectionalIterator world_get_directional_iterator(World* world, BoundingBox box);

typedef struct {
    WorldQuadrant* targetQuad;
    BoundingBox bounds;
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
    Vector3i tile_pos;
    WorldTile* data;
    int index;
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

YADF_API Vector3ic* chunk_get_position(WorldChunk* chunk);


#endif //YADF_WORLDAPI_H
