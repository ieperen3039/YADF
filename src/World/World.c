//
// Created by ieperen3039 on 27-11-19.
//

#include <assert.h>
#include "World.h"

struct _WorldQuadrant {
    Vector3i middle_pos; // coordinate of center, as the negative corner of tile (0, 0, 0))
    union { // if (height == 0) use leaves else use childs
        WorldQuadrant* childs[8];
        WorldChunk* leaves[8];
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

static WorldQuadrant* new_quadrant(AllocatorSM* allocator, WorldQuadrant* parent, int index) {
    WorldQuadrant* elt = allocator_sm_alloc(allocator);

    elt->parent = parent;
    elt->height = parent->height - 1;

    // we first take the middle of index 8
    int quad_reach = (CHUNK_LENGTH << (elt->height + 1)); // aka quadrant size, half of the parent quadrant size
    Vector3i middle = parent->middle_pos; // copy
    middle.x += quad_reach / 2;
    middle.y += quad_reach / 2;
    middle.z += quad_reach / 2;
    // then remove one quadrant size for each lower dimension
    if ((index & 1) == 0) middle.x -= quad_reach;
    if ((index & 2) == 0) middle.y -= quad_reach;
    if ((index & 4) == 0) middle.z -= quad_reach;
    elt->middle_pos = middle;
    // set leaves on NULL
    for (int i = 0; i < 8; ++i) {
        elt->leaves[i] = NULL;
    }
    return elt;
}

static void init_quadrant_leaf(WorldQuadrant* quadrant) {
    WorldChunk* chunks = malloc(8 * sizeof(WorldChunk));
    size_t offset = sizeof(WorldChunk);

    for (int i = 0; i < 8; ++i) {
        WorldChunk* chunk_section = (WorldChunk*) chunks + i * offset;

        chunk_section->parent = quadrant;
        Vector3i pos = quadrant->middle_pos;
        if ((i & 1) == 0) pos.x -= CHUNK_LENGTH;
        if ((i & 2) == 0) pos.y -= CHUNK_LENGTH;
        if ((i & 4) == 0) pos.z -= CHUNK_LENGTH;
        chunk_section->zero_pos = pos;

        quadrant->leaves[i] = chunk_section;
    }
}

PURE static inline bool quad_contains(WorldQuadrant* quadrant, Vector3i pos) {
    assert(quadrant != NULL);
    int half_quad_reach = (CHUNK_LENGTH << quadrant->height);
    Vector3i mid = quadrant->middle_pos;
    return
            pos.x < mid.x + half_quad_reach && pos.x > mid.x - half_quad_reach &&
            pos.y < mid.y + half_quad_reach && pos.y > mid.y - half_quad_reach &&
            pos.z < mid.z + half_quad_reach && pos.z > mid.z - half_quad_reach;
}

/// index of a quadrant or chunk in the child list of a quadrant
PURE static inline int get_quad_index(WorldQuadrant* parent, Vector3i coord) {
    assert(parent != NULL);
    Vector3i o = parent->middle_pos;
    int index = 0;
    if (coord.x > o.x) index &= 1;
    if (coord.y > o.y) index &= 2;
    if (coord.z > o.z) index &= 4;
    return index;
}

static WorldChunk* get_chunk_under_quad(WorldQuadrant* quadrant, Vector3i coord) {
    while (quadrant->height > 0) {
        int quad_ind = get_quad_index(quadrant, coord);
        WorldQuadrant* nextQuad = quadrant->childs[quad_ind];
        if (nextQuad == NULL) {
            return NULL;
        }
        quadrant = nextQuad;
    }

    return quadrant->leaves[get_quad_index(quadrant, coord)];
}

/// if quad_allocator == NULL and the quadrant does not exist, return NULL
WorldQuadrant* get_quad_under_quad(WorldQuadrant* quadrant, Vector3i tgtCoord, AllocatorSM* allocator) {
    assert(quadrant != NULL);

    while (quadrant->height > 0) {
        int quad_ind = get_quad_index(quadrant, tgtCoord);
        WorldQuadrant* nextQuad = quadrant->childs[quad_ind];
        if (nextQuad == NULL) {
            if (allocator == NULL) return NULL;
            // focus has updated, but chunk is undefined. Return NULL.
            quadrant = new_quadrant(allocator, quadrant, quad_ind);
        } else {
            quadrant = nextQuad;
        }
    }
    return quadrant;
}

/// if quad_allocator == NULL and the quadrant does not exist, return NULL
WorldQuadrant* get_quad_from_quad(WorldQuadrant* quadrant, Vector3i tgtCoord, AllocatorSM* quad_allocator) {
    assert(quadrant != NULL);

    while (!quad_contains(quadrant, tgtCoord)) {
        quadrant = quadrant->parent;
    }

    return get_quad_under_quad(quadrant, tgtCoord, quad_allocator);
}

WorldTileIterator world_get_tile_iterator(WorldChunk* chunk) {
    assert(chunk != NULL);
    return (WorldTileIterator) {
            chunk->zero_pos, chunk->tiles, 0
    };
}

WorldChunkIterator world_get_chunk_iterator(World* world, BoundingBox box) {
    return (WorldChunkIterator) {world->chunks, box, (Vector3i) {box.xMin, box.yMin, box.zMin}};
}

WorldChunk* world_chunk_iterator_next(WorldChunkIterator* itr) {
    Vector3i tgtCoord = itr->focus;

    // update focus
    itr->focus.x += CHUNK_LENGTH;
    if (itr->focus.x > itr->bounds.xMax) {
        itr->focus.x = itr->bounds.xMin;
        itr->focus.y += CHUNK_LENGTH;
        if (++itr->focus.y > itr->bounds.yMax) {
            itr->focus.y = itr->bounds.yMin;
            itr->focus.z += CHUNK_LENGTH;
        }
    }

    itr->targetQuad = get_quad_from_quad(itr->targetQuad, tgtCoord, NULL);
    if (itr->targetQuad == NULL) return NULL;

    int chunk_index = get_quad_index(itr->targetQuad, tgtCoord);
    return itr->targetQuad->leaves[chunk_index];
}

bool world_chunk_iterator_has_next(WorldChunkIterator* itr) {
    return itr->focus.z > itr->bounds.zMax;
}

WorldTileData world_tile_iterator_next(WorldTileIterator* itr) {
    int x = itr->chunk_pos.x + itr->index % CHUNK_LENGTH;
    int y = itr->chunk_pos.y + (itr->index % (CHUNK_LENGTH * CHUNK_LENGTH)) / CHUNK_LENGTH;
    int z = itr->chunk_pos.z + itr->index / (CHUNK_LENGTH * CHUNK_LENGTH);

    int i = itr->index++;
    return (WorldTileData) {
            &(itr->data[i]), {x, y, z}
    };
}

bool world_tile_iterator_has_next(WorldTileIterator* itr) {
    return itr->index < (CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_LENGTH);
}

PURE WorldChunk* get_chunk_from_quad(WorldQuadrant* quadrant, Vector3i coord) {
    assert(quadrant != NULL);
    while (!quad_contains(quadrant, coord)) quadrant = quadrant->parent;
    return get_chunk_under_quad(quadrant, coord);
}

PURE WorldTile* world_get_tile_from_chunk(WorldChunk* chunk, Vector3i coord) {
    assert(chunk != NULL);
    Vector3i c_zero = chunk->zero_pos;

    bool contains_pos = coord.z > c_zero.z && coord.y > c_zero.y && coord.x > c_zero.x &&
                        coord.x < c_zero.x + CHUNK_LENGTH &&
                        coord.y < c_zero.y + CHUNK_LENGTH &&
                        coord.z < c_zero.z + CHUNK_LENGTH;

    if (!contains_pos) {
        chunk = get_chunk_from_quad(chunk->parent, coord);
    }

    return &(chunk->grid[coord.x][coord.y][coord.z]);
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

void world_initialize_area(World* world, const BoundingBox area) {
    WorldQuadrant* quadrant = get_quad_under_quad(
            world->chunks, (Vector3i) {area.xMin, area.yMin, area.zMin}, world->quad_allocator
    );

    for (int x = area.xMin; x < area.xMax; x += CHUNK_LENGTH * 2) {
        for (int y = area.yMin; y < area.yMax; y += CHUNK_LENGTH * 2) {
            for (int z = area.zMin; z < area.zMax; z += CHUNK_LENGTH * 2) {
                quadrant = get_quad_from_quad(quadrant, (Vector3i) {x, y, z}, world->quad_allocator);
            }
        }
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
    // set children on NULL
    for (int i = 0; i < 8; ++i) {
        elt->childs[i] = NULL;
    }

    new_world->chunks = elt;
    new_world->depth = depth;

    return new_world;
}