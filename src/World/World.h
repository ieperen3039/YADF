//
// Created by ieperen3039 on 27-11-19.
//

#ifndef YADF2_WORLD_H
#define YADF2_WORLD_H
#include <stdlib.h>

#include "Structs.h"
#include "../DataStructures/List.h"
#include "../DataStructures/StaticMonoAllocator.h"
#include "../Materials.h"

#define CHUNK_COORD_BITS 4
#define CHUNK_LENGTH (1 << CHUNK_COORD_BITS)

typedef struct {
    struct {
        // items on this tile
        List item_ptrs;
        // true iff light may pass
        bool is_opaque;
        // material of the tile
        enum Material material;
        // temperature of the material
        float temperature; // in Kelvins
    } state;
    struct {

    } update;
} WorldTile;

typedef struct _WorldQuadrant WorldQuadrant;
typedef struct _WorldChunk WorldChunk;
typedef struct _World World;

// accepts a tile and the coordinate of that tile
typedef void(*TileFunction)(WorldTile, int x, int y, int z);

/**
 * Creates a new world, without any chunks initialized, of size `world_min_size`, around (0,0,0)
 * @param world_min_size the minimum size of this world in one dimension in nr of tiles
 * @return the new world, without any chunks initialized
 */
World* world_new(int world_min_size);

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

typedef struct {
    Vector3i chunk_pos;
    WorldTile* data;
    int index;
} WorldTileIterator;

WorldTileIterator world_get_tile_iterator(WorldChunk* chunk);;

typedef struct {
    WorldTile* elt;
    Vector3i coord;
} WorldTileData;

WorldTileData world_tile_iterator_next(WorldTileIterator* itr);

bool world_tile_iterator_has_next(WorldTileIterator* itr);

typedef struct {
    WorldQuadrant* targetQuad;
    BoundingBox bounds;
    Vector3i focus;
} WorldChunkIterator;

/**
 * creates an iterator of at least the chunks that include the coordinates of the bounding box
 */
WorldChunkIterator world_get_chunk_iterator(World* world, BoundingBox box);

/** returns the next chunk. Returns NULL if the chunk is not initialized */
WorldChunk* world_chunk_iterator_next(WorldChunkIterator* itr);

bool world_chunk_iterator_has_next(WorldChunkIterator* itr);

#endif //YADF2_WORLD_H
