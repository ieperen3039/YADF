//
// Created by ieperen3039 on 27-11-19.
//

#include "World.h"
#include "../DataStructures/StaticMonoAllocator.h"
#include <assert.h>
#include <WorldAPI.h>


struct _Entity;

struct _WorldQuadrant {
    Vector3i middle_pos; // coordinate of center, as the negative corner of tile (0, 0, 0))
    union { // if (height == 0) use leaves else use childs
        WorldQuadrant* childs[8];
        WorldChunk* leaves[8];
    };
    WorldQuadrant* parent;
    unsigned char height;
};

struct _World {
    WorldQuadrant* chunks;
    AllocatorSM* quad_allocator;
    unsigned char depth;

    List entities_to_update;
};

struct _WorldChunk {
    union {
        WorldTile tiles[CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_LENGTH];
        WorldTile grid[CHUNK_LENGTH][CHUNK_LENGTH][CHUNK_LENGTH];
    };
    // parent quadrant of this chunk
    WorldQuadrant* parent;
    // position of grid[0][0][0]
    Vector3i zero_pos;
};

WorldQuadrant* new_quadrant(AllocatorSM* allocator, WorldQuadrant* parent, int index) {
    WorldQuadrant* elt = allocator_sm_alloc(allocator);

    parent->childs[index] = elt;
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

void init_quadrant_leaf(WorldQuadrant* quadrant, const WorldTile initial_tile) {
    WorldChunk* chunks = malloc(8 * sizeof(WorldChunk));

    for (int i = 0; i < 8; ++i) {
        WorldChunk* chunk_section = (WorldChunk*) chunks + i;

        chunk_section->parent = quadrant;
        Vector3i pos = quadrant->middle_pos;
        if ((i & 1) == 0) pos.x -= CHUNK_LENGTH;
        if ((i & 2) == 0) pos.y -= CHUNK_LENGTH;
        if ((i & 4) == 0) pos.z -= CHUNK_LENGTH;
        chunk_section->zero_pos = pos;

        for (int j = 0; j < (CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_LENGTH); ++j) {
            chunk_section->tiles[j] = initial_tile;
            // we are not going to copy the entities, so clear the list to be sure
            if (list_get_size(&initial_tile.entity_ptrs) > 0) {
                list_clear(&chunk_section->tiles[j].entity_ptrs);
            }
        }

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
PURE static inline int get_quad_index(WorldQuadrant* parent, Vector3ic* coord) {
    assert(parent != NULL);
    Vector3i o = parent->middle_pos;
    int index = 0;
    if (coord->x > o.x) index |= 1;
    if (coord->y > o.y) index |= 2;
    if (coord->z > o.z) index |= 4;
    return index;
}

/// if quad_allocator == NULL and the quadrant does not exist, return NULL
WorldQuadrant* get_quad_under_quad(WorldQuadrant* quadrant, Vector3ic* tgtCoord, AllocatorSM* allocator) {
    assert(quadrant != NULL);
    assert(quad_contains(quadrant, *tgtCoord));

    while (quadrant->height > 0) {
        int quad_ind = get_quad_index(quadrant, tgtCoord);
        WorldQuadrant* nextQuad = quadrant->childs[quad_ind];

        if (nextQuad == NULL) {
            if (allocator == NULL) return NULL;
            quadrant = new_quadrant(allocator, quadrant, quad_ind);
        } else {
            quadrant = nextQuad;
        }
    }

    return quadrant;
}

/// if quad_allocator == NULL and the quadrant does not exist, return NULL
static WorldChunk* get_chunk_under_quad(WorldQuadrant* quadrant, Vector3ic* coord, AllocatorSM* allocator) {
    assert(quad_contains(quadrant, *coord));
    quadrant = get_quad_under_quad(quadrant, coord, allocator);

    return quadrant->leaves[get_quad_index(quadrant, coord)];
}

WorldTile* get_tile_under_quad(WorldQuadrant* quadrant, Vector3ic* coord) {
    assert(quad_contains(quadrant, *coord));
    WorldChunk* chunk = get_chunk_under_quad(quadrant, coord, NULL);

    return &(chunk->grid[coord->x % CHUNK_LENGTH][coord->y % CHUNK_LENGTH][coord->z % CHUNK_LENGTH]);
}

/**
 * returns the leaf quadrant containing the given coordinate.
 * Searching starts from the given quadrant.
 * if quad_allocator == NULL and the quadrant does not exist, return NULL
 * @param quadrant
 * @param tgtCoord
 * @param quad_allocator
 * @return the leaf quadrant containing the coordinate, or NULL if it can't be found nor made
 */
WorldQuadrant* get_quad_from_quad(WorldQuadrant* quadrant, Vector3ic* tgtCoord, AllocatorSM* quad_allocator) {
    assert(quadrant != NULL);

    while (!quad_contains(quadrant, *tgtCoord)) {
        quadrant = quadrant->parent;
        if (quadrant == NULL) return NULL;
    }

    return get_quad_under_quad(quadrant, tgtCoord, quad_allocator);
}

WorldChunkIterator world_get_chunk_iterator(World* world, BoundingBox box) {
    return (WorldChunkIterator) {world->chunks, box, (Vector3i) {box.xMin, box.yMin, box.zMin}};
}

/** @return the zero_pos of the chunk containing the given world coordinate */
Vector3i chunk_get_zero(Vector3i* tile_coord){
    const int xRem = tile_coord->x % CHUNK_LENGTH;
    const int yRem = tile_coord->y % CHUNK_LENGTH;
    const int zRem = tile_coord->z % CHUNK_LENGTH;

    return (Vector3i){
        (tile_coord->x - xRem),
        (tile_coord->y - yRem),
        (tile_coord->z - zRem)
    };
}

WorldChunkData world_chunk_iterator_next(WorldChunkIterator* itr) {
    Vector3i zeroPos = chunk_get_zero(&itr->focus);

    // update focus
    itr->focus.x += CHUNK_LENGTH;
    if (itr->focus.x > itr->bounds.xMax) {
        itr->focus.x = itr->bounds.xMin;
        itr->focus.y += CHUNK_LENGTH;
        if (itr->focus.y > itr->bounds.yMax) {
            itr->focus.y = itr->bounds.yMin;
            itr->focus.z += CHUNK_LENGTH;
        }
    }

    WorldQuadrant* new_quad = get_quad_from_quad(itr->targetQuad, &zeroPos, NULL);
    if (new_quad == NULL) return (WorldChunkData) {NULL, zeroPos};

    int chunk_index = get_quad_index(new_quad, &zeroPos);
    WorldChunk* pChunk = new_quad->leaves[chunk_index];
    itr->targetQuad = new_quad;

    return (WorldChunkData) {pChunk, zeroPos}; // may be NULL
}

PURE bool world_chunk_iterator_has_next(WorldChunkIterator* itr) {
    return itr->focus.z < itr->bounds.zMax;
}

PURE WorldTileIterator chunk_get_tile_iterator(WorldChunk* chunk) {
    assert(chunk != NULL);
    return (WorldTileIterator) {
            chunk->zero_pos, chunk->zero_pos, chunk->tiles, 0
    };
}

WorldTileData chunk_tile_iterator_next(WorldTileIterator* itr) {
    int i = itr->index++;
    WorldTile* tile = &(itr->data[i]);
    WorldTileData data = {tile, itr->tile_pos};

    itr->tile_pos.x++;
    if (itr->tile_pos.x > CHUNK_LENGTH) {
        itr->tile_pos.x = itr->chunk_pos.x;
        itr->tile_pos.y++;
        if (itr->tile_pos.y > CHUNK_LENGTH) {
            itr->tile_pos.y = itr->chunk_pos.y;
            itr->tile_pos.z++;
        }
    }

    return data;
}

PURE bool chunk_tile_iterator_has_next(WorldTileIterator* itr) {
    return itr->index < (CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_LENGTH);
}

PURE WorldChunk* get_chunk_from_quad(WorldQuadrant* quadrant, Vector3ic* coord, AllocatorSM* allocator) {
    assert(quadrant != NULL);
    while (!quad_contains(quadrant, *coord)) quadrant = quadrant->parent;
    return get_chunk_under_quad(quadrant, coord, allocator);
}

PURE WorldTile* world_get_tile_from_chunk(WorldChunk* chunk, Vector3ic* coord) {
    assert(chunk != NULL);
    Vector3i c_zero = chunk->zero_pos;

    bool contains_pos = coord->z > c_zero.z && coord->y > c_zero.y && coord->x > c_zero.x &&
                        coord->x < c_zero.x + CHUNK_LENGTH &&
                        coord->y < c_zero.y + CHUNK_LENGTH &&
                        coord->z < c_zero.z + CHUNK_LENGTH;

    if (!contains_pos) {
        chunk = get_chunk_from_quad(chunk->parent, coord, NULL);
        if (chunk == NULL) return NULL;
    }

    return &(chunk->grid[coord->x % CHUNK_LENGTH][coord->y % CHUNK_LENGTH][coord->z % CHUNK_LENGTH]);
}

int world_initialize_area(World* world, const BoundingBox area, const WorldTile initial_tile) {
    int initialized_chunks = 0;

    Vector3i coord = (Vector3i) {area.xMin, area.yMin, area.zMin};
    WorldQuadrant* quadrant = get_quad_under_quad(
            world->chunks, &coord, world->quad_allocator
    );

    // extend initialisation range to include upper bounds
    int xMax = area.xMax + (CHUNK_LENGTH * 2);
    int yMax = area.yMax + (CHUNK_LENGTH * 2);
    int zMax = area.zMax + (CHUNK_LENGTH * 2);

    for (int x = area.xMin; x < xMax; x += CHUNK_LENGTH * 2) {
        for (int y = area.yMin; y < yMax; y += CHUNK_LENGTH * 2) {
            for (int z = area.zMin; z < zMax; z += CHUNK_LENGTH * 2) {
                Vector3i tgt_coord = (Vector3i) {x, y, z};
                quadrant = get_quad_from_quad(quadrant, &tgt_coord, world->quad_allocator);

                if (quadrant->leaves[0] == NULL) {
                    init_quadrant_leaf(quadrant, initial_tile);
                    initialized_chunks += 4;
                }
            }
        }
    }

    return initialized_chunks;
}

World* world_new(int world_min_size) {
    World* new_world = malloc(sizeof(World));

    // calculate world depth
    int depth = 0;
    world_min_size >>= CHUNK_COORD_BITS;
    int size_in_chunks = 1;
    while (size_in_chunks <= world_min_size) {
        size_in_chunks *= 2;
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
    list_init(&new_world->entities_to_update, sizeof(struct _Entity*), WORLD_DEFAULT_ENTITY_LIST_CAPACITY);
    LOG_INFO_F("Created world with %d layers", new_world->depth);

    return new_world;
}

void free_quad(WorldQuadrant* quad) {
    if (quad == NULL) return;

    if (quad->height == 0) {
        free(quad->leaves); // chunks are allocated per 8

    } else {
        for (int i = 0; i < 8; ++i) {
            free_quad(quad->childs[i]);
        }
    }

    // quad itself is allocated in world.quad_allocator
}

void world_free(World* world) {
    free_quad(world->chunks);
    allocator_sm_free(world->quad_allocator);
    free(world);
}

PURE List* world_get_entities_to_update(World* world) {
    return &world->entities_to_update;
}

Vector3ic* chunk_get_position(WorldChunk* chunk) {
    return &chunk->zero_pos;
}

PURE Vector3i to_int_inflate(const Vector3f* target) {
    char xInc = target->x < 0 ? -1 : 1;
    char yInc = target->y < 0 ? -1 : 1;
    char zInc = target->z < 0 ? -1 : 1;

    return (Vector3i) {
            target->x + xInc,
            target->y + yInc,
            target->z + zInc
    };
}

WorldDirectionalIterator world_directional_iterator(
        const World* world, const Vector3f* focus, const Vector3f* size, bool x_pos, bool y_pos
) {
    Vector3i origin = to_int_inflate(focus);
    Vector3i size_i = to_int_inflate(size);
    origin.x -= size_i.x;
    origin.y -= size_i.y;
    origin.z -= size_i.z;

    return (WorldDirectionalIterator) {
            world, get_quad_under_quad(world->chunks, &origin, NULL),
            origin, size_i, 0, 0, 0, x_pos, y_pos, false
    };
}

WorldTileData world_directional_iterator_next(WorldDirectionalIterator* itr) {
    const char xInc = itr->xp ? 1 : -1;
    const char yInc = itr->yp ? 1 : -1;
    const char zInc = -1;
    const int xOff = itr->xSteps * xInc;
    const int yOff = itr->ySteps * yInc;
    const int zOff = itr->zSteps * zInc;

    // 4 skews:
    // add  y to x
    // add -x to y
    // add -y to z
    // add y to x and y
    Vector3i position = itr->origin;
    position.x += xOff + yOff + yOff;
    position.y += yOff - xOff + yOff;
    position.z += zOff - yOff;
    // these skews creates one hole behind each vertex
    if (itr->isSecondary) position.x += xInc;

    WorldTile* tile = NULL;
    if (!quad_contains(itr->quad, position)) {
        WorldQuadrant* new_quad = get_quad_from_quad(itr->quad, &position, NULL);

        if (new_quad != NULL) {
            tile = get_tile_under_quad(new_quad, &position);
            itr->quad = new_quad;
        }
    } else {
        tile = get_tile_under_quad(itr->quad, &position);
    }

    WorldTileData tile_data = {tile, position};

    itr->xSteps++;
    if (itr->xSteps > itr->size.x) {
        itr->xSteps = 0;

        itr->ySteps++;
        if (itr->ySteps > itr->size.y) {
            itr->ySteps = itr->zSteps; // results in a maximum z-level

            if (itr->isSecondary) {
                itr->zSteps++;
                itr->isSecondary = false;

            } else {
                itr->isSecondary = true;
            }
        }
    }

    return tile_data;
}

WorldTile* world_get_tile(World* world, Vector3i* coord){
    return get_tile_under_quad(world->chunks, coord);
}

bool world_directional_iterator_has_next(WorldDirectionalIterator* itr) {
    return itr->zSteps > itr->size.z;
}

void world_tile_init(WorldTile* base_tile, char flags) {
    base_tile->flags = flags;
    list_init(&base_tile->entity_ptrs, sizeof(void*), 0);
}
