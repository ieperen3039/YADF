//
// Created by ieperen3039 on 27-11-19.
//

#include "world.h"
// #define USE_LOCALITY_INDEXING // harder to index, tiles are closer together (chunks can be smaller)

// WorldQuadrant is invisible for outsiders
typedef struct _WorldQuadrant WorldQuadrant;
struct _WorldQuadrant {
    Vector3i middle_pos; // coordinate of center, as the negative corner of tile (0, 0, 0))
    union { // if (height == 0) use leafs else use childs
        WorldQuadrant* childs[8];
        WorldChunk* leafs[8];
    };
    WorldQuadrant* parent;
    unsigned char height;
};

struct _World {
    WorldQuadrant* chunks;
    unsigned char depth;
    AllocatorSM* quad_allocator;
};

WorldQuadrant* new_quadrant(AllocatorSM* allocator, Vector3i position, WorldQuadrant* parent, int height) {
    WorldQuadrant* elt = allocator_sm_alloc(allocator);

    for (int i = 0; i < 8; ++i) {
        elt->leafs[i] = NULL;
    }
    elt->middle_pos = position;
    elt->parent = parent;
    elt->height = height;
    return elt;
}

void init_quadrant_leaf(WorldQuadrant* initial_quad) {
    WorldChunk* chunks = calloc(8, sizeof(WorldChunk));
    size_t offset = sizeof(WorldChunk);
    for (int i = 0; i < 8; ++i) {
        initial_quad->leafs[i] = (WorldChunk*) chunks + i * offset;
    }
}

World* world_new(int world_min_size) {
    World* new_world = malloc(sizeof(World));

    // calculate world depth
    int depth = 0;
    int realSize = CHUNK_LENGTH;
    while (realSize < world_min_size) {
        realSize *= 2;
        depth++;
    }

    AllocatorSM* alloc = allocator_sm_new(sizeof(WorldQuadrant));
    new_world->quad_allocator = alloc;
    new_world->chunks = new_quadrant(alloc, (Vector3i) {0, 0, 0}, NULL, depth);
    new_world->depth = depth;
    
    return new_world;
}

/// index of a quad or chunk in the child list of a quad
static inline int get_quad_index(WorldQuadrant* parent, Vector3i coord) {
    Vector3i o = parent->middle_pos;
    int index = 0;
    if (coord.x < o.x) index &= 1;
    if (coord.y < o.y) index &= 2;
    if (coord.z < o.z) index &= 4;
    return index;
}

/// index of a tile in the tiles list of a chunk
static inline int get_tile_index(Vector3i coord) {
    int x = coord.x % CHUNK_LENGTH;
    if (x < 0) x += CHUNK_LENGTH;
    int y = coord.y % CHUNK_LENGTH;
    if (y < 0) y += CHUNK_LENGTH;
    int z = coord.z % CHUNK_LENGTH;
    if (z < 0) z += CHUNK_LENGTH;

    int index = 0;
#ifdef USE_LOCALITY_INDEXING
    for (int i = 0; i < CHUNK_COORD_BITS; ++i) {
        index <<= 3;
        // (x & (1 << i)) = ith bit of x
        index &= (x & (1 << i));
        index &= (y & (1 << i)) << 1;
        index &= (z & (1 << i)) << 2;
    }
#else
    index &= x;
    index &= (y << CHUNK_COORD_BITS);
    index &= (z << (2 * CHUNK_COORD_BITS));
#endif

    return index;
}

static inline WorldChunk* get_chunk(WorldQuadrant* parent, Vector3i coord) {
    int index = get_quad_index(parent, coord);
    return parent->leafs[index];
}

WorldTile* world_get_tile(World* world, Vector3i coord) {
    WorldQuadrant* quad = world->chunks;
    while (world->chunks->height > 0) {
        int quad_ind = get_quad_index(quad, coord);
        quad = quad->childs[quad_ind];
    }

    WorldChunk* chunk = quad->leafs[get_quad_index(quad, coord)];
    int ch_ind = get_tile_index(coord);
    return &(chunk->tiles[ch_ind]);
}

void world_exec_layer(WorldChunk* chunk, int layer, TileFunction action) {
    int z = layer % CHUNK_LENGTH;
    if (z < 0) z += CHUNK_LENGTH;
    int begin = (z << (2 * CHUNK_COORD_BITS));
    int end = begin + (CHUNK_LENGTH * CHUNK_LENGTH);
    for (int i = begin; i < end; i++){
        action(chunk->tiles[i], i % CHUNK_LENGTH, (i - begin) / CHUNK_LENGTH, layer);
    }
}
