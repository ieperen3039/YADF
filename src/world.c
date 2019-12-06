//
// Created by ieperen3039 on 27-11-19.
//

#include "world.h"
#define LOCALITY_INDEXING false

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
    int depth = 0;
    while (world_min_size > CHUNK_SIZE) {
        world_min_size /= 2;
        depth++;
    }
    AllocatorSM* alloc = allocator_sm_new(sizeof(WorldQuadrant));
    new_world->quad_allocator = alloc;
    new_world->chunks = new_quadrant(alloc, (Vector3i) {0, 0, 0}, NULL, depth);
    new_world->depth = depth;
    
    return new_world;
}

static inline int get_quad_index(WorldQuadrant* parent, Vector3i coord) {
    Vector3i o = parent->middle_pos;
    int index = 0;
    if (coord.x < o.x) index &= 1;
    if (coord.y < o.y) index &= 2;
    if (coord.z < o.z) index &= 4;
    return index;
}

static inline int get_chunk_index(Vector3i coord) {
    int x = coord.x % CHUNK_SIZE;
    if (x < 0) x += CHUNK_SIZE;
    int y = coord.y % CHUNK_SIZE;
    if (y < 0) y += CHUNK_SIZE;
    int z = coord.z % CHUNK_SIZE;
    if (z < 0) z += CHUNK_SIZE;

    int index = 0;
    if (LOCALITY_INDEXING) {
        for (int i = 0; i < CHUNK_COORD_BITS; ++i) {
            // (x & (1 << i)) = ith bit of x
            // bit << (2 * i) = shift 3 times as far
            index &= ((x & (1 << i)) << ((3 * i)));
            index &= ((y & (1 << i)) << ((3 * i) + 1));
            index &= ((z & (1 << i)) << ((3 * i) + 2));
        }
    } else {
        index &= x;
        index &= (y << CHUNK_COORD_BITS);
        index &= (z << (2 * CHUNK_COORD_BITS));
    }

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
    int ch_ind = get_chunk_index(coord);
    return &(chunk->tiles[ch_ind]);
}

void world_render_layer(World* world, int layer) {

}
