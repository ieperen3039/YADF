//
// Created by ieperen3039 on 27-11-19.
//

#include "World.h"
// #define USE_LOCALITY_INDEXING // harder to index, tiles are closer together (chunks can be smaller)

struct _WorldQuadrant {
    Vector3i middle_pos; // coordinate of center, as the negative corner of tile (0, 0, 0))
    union { // if (height == 0) use leafs else use childs
        WorldQuadrant* childs[8];
        WorldChunk* leafs[8];
    };
    WorldQuadrant* parent;
    unsigned char height;
};

struct _WorldChunk {
    union {
        WorldTile tiles[CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_LENGTH];
        WorldTile grid[CHUNK_LENGTH][CHUNK_LENGTH][CHUNK_LENGTH];
    };
    WorldQuadrant* parent;
    Vector3i zero_pos;
};

struct _World {
    WorldQuadrant* chunks;
    unsigned char depth;
    AllocatorSM* quad_allocator;
};

static WorldTile* get_tile_under_quad(Vector3i coord, WorldQuadrant* quad, AllocatorSM* quad_allocator);

static WorldQuadrant* new_quadrant(AllocatorSM* allocator, WorldQuadrant* parent, int index) {
    WorldQuadrant* elt = allocator_sm_alloc(allocator);

    elt->parent = parent;
    elt->height = parent->height - 1;

    // we first take the middle of index 8
    int quad_reach = (CHUNK_LENGTH << (elt->height + 1)); // aka quad size, half of the parent quad size
    Vector3i middle = parent->middle_pos; // copy
    middle.x += quad_reach / 2;
    middle.y += quad_reach / 2;
    middle.z += quad_reach / 2;
    // then remove one quad size for each lower dimension
    if ((index & 1) == 0) middle.x -= quad_reach;
    if ((index & 2) == 0) middle.y -= quad_reach;
    if ((index & 4) == 0) middle.z -= quad_reach;
    elt->middle_pos = middle;
    // set leafs on NULL
    for (int i = 0; i < 8; ++i) {
        elt->leafs[i] = NULL;
    }
    return elt;
}

static void init_quadrant_leaf(WorldQuadrant* quad) {
    WorldChunk* chunks = malloc(8 * sizeof(WorldChunk));
    size_t offset = sizeof(WorldChunk);

    for (int i = 0; i < 8; ++i) {
        WorldChunk* chunk_section = (WorldChunk*) chunks + i * offset;

        chunk_section->parent = quad;
        Vector3i pos = quad->middle_pos;
        if ((i & 1) == 0) pos.x -= CHUNK_LENGTH;
        if ((i & 2) == 0) pos.y -= CHUNK_LENGTH;
        if ((i & 4) == 0) pos.z -= CHUNK_LENGTH;
        chunk_section->zero_pos = pos;

        quad->leafs[i] = chunk_section;
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

    WorldQuadrant* elt = allocator_sm_alloc(alloc);
    // literal initialisation of -special case- root quadrant
    elt->parent = NULL;
    elt->height = depth; // height of root == depth of tree
    elt->middle_pos = (Vector3i) {0, 0, 0};
    // set leafs on NULL
    for (int i = 0; i < 8; ++i) {
        elt->leafs[i] = NULL;
    }

    new_world->chunks = elt;
    new_world->depth = depth;

    return new_world;
}

static bool quad_contains(WorldQuadrant* quadrant, Vector3i pos) {
    int half_quad_reach = (CHUNK_LENGTH << quadrant->height);
    Vector3i mid = quadrant->middle_pos;
    return
            pos.x < mid.x + half_quad_reach && pos.x > mid.x - half_quad_reach &&
            pos.y < mid.y + half_quad_reach && pos.y > mid.y - half_quad_reach &&
            pos.z < mid.z + half_quad_reach && pos.z > mid.z - half_quad_reach;
}

/// index of a quad or chunk in the child list of a quad
static inline int get_quad_index(WorldQuadrant* parent, Vector3i coord) {
    Vector3i o = parent->middle_pos;
    int index = 0;
    if (coord.x > o.x) index &= 1;
    if (coord.y > o.y) index &= 2;
    if (coord.z > o.z) index &= 4;
    return index;
}

PURE static inline WorldChunk* get_chunk(WorldQuadrant* parent, Vector3i coord) {
    int index = get_quad_index(parent, coord);
    return parent->leafs[index];
}

WorldTile* world_get_tile(World* world, Vector3i coord) {
    return get_tile_under_quad(coord, world->chunks, world->quad_allocator);
}

/** Get the tile on the given coordinate that is known to be a child of the given quad */
static WorldTile* get_tile_under_quad(Vector3i coord, WorldQuadrant* quad, AllocatorSM* quad_allocator) {
    while (quad->height > 0) {
        int quad_ind = get_quad_index(quad, coord);
        WorldQuadrant* nextQuad = quad->childs[quad_ind];
        if (nextQuad == NULL) {
            if (quad_allocator == NULL) return NULL;
            nextQuad = new_quadrant(quad_allocator, quad, quad_ind);
            quad->childs[quad_ind] = nextQuad;
        }
        quad = nextQuad;
    }

    WorldChunk* chunk = quad->leafs[get_quad_index(quad, coord)];
    return &(chunk->grid[coord.x][coord.y][coord.z]);
}

WorldTileIterator world_get_tile_iterator(WorldChunk* chunk) {
    return (WorldTileIterator) {
            chunk->zero_pos, chunk->tiles, 0
    };
}

WorldChunkIterator world_get_chunk_iterator(World* world) {
    return (WorldChunkIterator){world->chunks, 0, world->depth};
}

WorldChunk* world_chunk_iterator_next(WorldChunkIterator* itr) {
    // this can be optimized further by only checking parent chunk of current iff mask & index == 0
    int depth = itr->world_depth;
    unsigned char mask = 0b0111 << depth * 3;
    int index = itr->index++;

    WorldQuadrant* quad = itr->root;
    while (depth > 0) {
        int i = (index & mask) >> (depth * 3);
        quad = quad->childs[i];
        index = index &~ mask; // index without mask
        depth--;
    }

    return (WorldChunk*) quad; // oh it was actually a chunk
}

WorldTileData world_tile_iterator_next(WorldTileIterator* itr) {
    int x = itr->index % CHUNK_LENGTH;
    int y = (itr->index % (CHUNK_LENGTH * CHUNK_LENGTH)) / CHUNK_LENGTH;
    int z = itr->index / (CHUNK_LENGTH * CHUNK_LENGTH);

    int i = itr->index++;
    return (WorldTileData) {
            &(itr->data[i]), {x, y, z}
    };
}

PURE WorldTile* world_get_tile_from_chunk(WorldChunk* chunk, Vector3i pos) {
    Vector3i c_zero = chunk->zero_pos;

    bool contains_pos = pos.z > c_zero.z && pos.y > c_zero.y && pos.x > c_zero.x &&
                        pos.x < c_zero.x + CHUNK_LENGTH &&
                        pos.y < c_zero.y + CHUNK_LENGTH &&
                        pos.z < c_zero.z + CHUNK_LENGTH;

    if (!contains_pos) {
        WorldQuadrant* quadrant = chunk->parent;
        while (!quad_contains(quadrant, pos)) quadrant = quadrant->parent;
        return get_tile_under_quad(pos, quadrant, NULL);
    }

    return &(chunk->grid[pos.x][pos.y][pos.z]);
}

void world_exec_layer(WorldChunk* chunk, int layer, TileFunction action) {
    int z = layer % CHUNK_LENGTH;
    if (z < 0) z += CHUNK_LENGTH;
    int begin = (z << (2 * CHUNK_COORD_BITS));
    int end = begin + (CHUNK_LENGTH * CHUNK_LENGTH);

    for (int i = begin; i < end; i++) {
        action(chunk->tiles[i], i % CHUNK_LENGTH, (i - begin) / CHUNK_LENGTH, layer);
    }
}
