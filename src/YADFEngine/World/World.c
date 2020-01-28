//
// Created by ieperen3039 on 27-11-19.
//

#include "World.h"
#include "../DataStructures/StaticMonoAllocator.h"
#include <assert.h>
#include <WorldAPI.h>
#include <Entity.h>
#include "../Entities/Fluids.h"
#include "../DataStructures/Map.h"

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
    List fluids_to_update;
};

#define index(x, y, z) ((x) + (y) * CHUNK_LENGTH + (z) * CHUNK_LENGTH * CHUNK_LENGTH)

struct _WorldChunk {
    // (x, y, z) is at tiles[x + y * CHUNK_LENGTH + z * CHUNK_LENGTH * CHUNK_LENGTH], == tiles[index(x, y, z)];
    WorldTile tiles[CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_LENGTH];
  ElementMap fluids;
  // parent quadrant of this chunk
  WorldQuadrant* parent;
  // position of grid[0][0][0]
  Vector3i zero_pos;
};

WorldQuadrant* new_quadrant(AllocatorSM* allocator, WorldQuadrant* parent, int index) {
    assert(parent->height > 0);
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

void init_tile(WorldTile* tile, int flags, int index) {
  tile->flags = flags;
  tile->index = index;
  list_init(&tile->entity_ptrs, sizeof(void*), 0);
}

void init_quadrant_leaf(WorldQuadrant* quadrant, int initial_flags) {
    WorldChunk* chunks = malloc(8 * sizeof(WorldChunk));

    for (int i = 0; i < 8; ++i) {
        WorldChunk* chunk_section = chunks + i;

        chunk_section->parent = quadrant;
        Vector3i pos = quadrant->middle_pos;
        if ((i & 1) == 0) pos.x -= CHUNK_LENGTH;
        if ((i & 2) == 0) pos.y -= CHUNK_LENGTH;
        if ((i & 4) == 0) pos.z -= CHUNK_LENGTH;
        chunk_section->zero_pos = pos;

        for (int j = 0; j < (CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_LENGTH); ++j) {
          WorldTile* tile = &chunk_section->tiles[j];
          init_tile(tile, initial_flags, j);
        }
        map_init(&chunk_section->fluids, sizeof(FluidFlow));

        quadrant->leaves[i] = chunk_section;
    }
}

PURE static inline bool quad_contains(WorldQuadrant* quadrant, Vector3i pos) {
    assert(quadrant != NULL);
    int half_quad_reach = (CHUNK_LENGTH << quadrant->height);
    Vector3i mid = quadrant->middle_pos;
    return
            pos.x < mid.x + half_quad_reach && pos.x >= mid.x - half_quad_reach &&
            pos.y < mid.y + half_quad_reach && pos.y >= mid.y - half_quad_reach &&
            pos.z < mid.z + half_quad_reach && pos.z >= mid.z - half_quad_reach;
}

/// index of a quadrant or chunk in the child list of a quadrant
PURE static inline int get_quad_index(WorldQuadrant* parent, Vector3ic* coord) {
    assert(parent != NULL);
    assert(quad_contains(parent, *coord));
    Vector3i o = parent->middle_pos;
    int index = 0;
    if (coord->x >= o.x) index |= 1;
    if (coord->y >= o.y) index |= 2;
    if (coord->z >= o.z) index |= 4;
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

    assert(quad_contains(quadrant, *tgtCoord));

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

    int xRem = coord->x % CHUNK_LENGTH;
    int yRem = coord->y % CHUNK_LENGTH;
    int zRem = coord->z % CHUNK_LENGTH;
    // correct for negative
    if (xRem < 0) xRem += CHUNK_LENGTH;
    if (yRem < 0) yRem += CHUNK_LENGTH;
    if (zRem < 0) zRem += CHUNK_LENGTH;

    return &(chunk->tiles[index(xRem, yRem, zRem)]);
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

/** @return the zero_pos of the chunk containing the given world coordinate */
Vector3i chunk_get_zero(Vector3i* tile_coord) {
    int xRem = tile_coord->x % CHUNK_LENGTH;
    int yRem = tile_coord->y % CHUNK_LENGTH;
    int zRem = tile_coord->z % CHUNK_LENGTH;
    // correct for negative
    if (xRem < 0) xRem += CHUNK_LENGTH;
    if (yRem < 0) yRem += CHUNK_LENGTH;
    if (zRem < 0) zRem += CHUNK_LENGTH;

    return (Vector3i) {
            (tile_coord->x - xRem),
            (tile_coord->y - yRem),
            (tile_coord->z - zRem)
    };
}

WorldChunkIterator world_get_chunk_iterator(World* world, BoundingBox box) {
    Vector3i zeroPos = (Vector3i) {box.xMin, box.yMin, box.zMin};
    zeroPos = chunk_get_zero(&zeroPos);
    return (WorldChunkIterator) {world->chunks, box, zeroPos, zeroPos};
}

WorldChunkData world_chunk_iterator_next(WorldChunkIterator* itr) {
    Vector3i zeroPos = itr->focus;

    itr->focus.x += CHUNK_LENGTH;
    if (itr->focus.x > itr->bounds.xMax) {
        itr->focus.x = itr->zero.x;
        itr->focus.y += CHUNK_LENGTH;
        if (itr->focus.y > itr->bounds.yMax) {
            itr->focus.y = itr->zero.y;
            itr->focus.z += CHUNK_LENGTH;
        }
    }

    WorldQuadrant* new_quad = get_quad_from_quad(itr->targetQuad, &zeroPos, NULL);
    if (new_quad == NULL) {
        return (WorldChunkData) {NULL, zeroPos};
    }

    int chunk_index = get_quad_index(new_quad, &zeroPos);
    WorldChunk* pChunk = new_quad->leaves[chunk_index];
    itr->targetQuad = new_quad;

    return (WorldChunkData) {pChunk, zeroPos}; // may be NULL
}

PURE bool world_chunk_iterator_has_next(WorldChunkIterator* itr) {
    return itr->focus.z <= itr->bounds.zMax;
}

PURE WorldTileIterator chunk_get_tile_iterator(WorldChunk* chunk) {
    assert(chunk != NULL);
    return (WorldTileIterator) {
            chunk->zero_pos, (Vector3i) {0, 0, 0}, chunk
    };
}

WorldTileData chunk_tile_iterator_next(WorldTileIterator* itr) {
    Vector3i pos = itr->tile_in_chunk_pos;

    itr->tile_in_chunk_pos.x++;
    if (itr->tile_in_chunk_pos.x >= CHUNK_LENGTH) {
        itr->tile_in_chunk_pos.x = 0;
        itr->tile_in_chunk_pos.y++;
        if (itr->tile_in_chunk_pos.y >= CHUNK_LENGTH) {
            itr->tile_in_chunk_pos.y = 0;
            itr->tile_in_chunk_pos.z++;
        }
    }

    Vector3i chunkPos = itr->chunk_pos;
    WorldTile* tile = &(itr->chunk->tiles[index(pos.x, pos.y, pos.z)]);
    WorldTileData data = {tile, {pos.x + chunkPos.x, pos.y + chunkPos.y, pos.z + chunkPos.z}};

    return data;
}

PURE bool chunk_tile_iterator_has_next(WorldTileIterator* itr) {
    return itr->tile_in_chunk_pos.z < CHUNK_LENGTH;
}

PURE WorldChunk* get_chunk_from_quad(WorldQuadrant* quadrant, Vector3ic* coord, AllocatorSM* allocator) {
    assert(quadrant != NULL);
    while (!quad_contains(quadrant, *coord)) quadrant = quadrant->parent;
    return get_chunk_under_quad(quadrant, coord, allocator);
}

bool chunk_contains(const WorldChunk* chunk, Vector3ic* coord) {
    Vector3i c_zero = chunk->zero_pos;
    return coord->z > c_zero.z && coord->y > c_zero.y && coord->x > c_zero.x &&
                        coord->x < c_zero.x + CHUNK_LENGTH &&
                        coord->y < c_zero.y + CHUNK_LENGTH &&
                        coord->z < c_zero.z + CHUNK_LENGTH;
}

PURE WorldTile* world_get_tile_from_chunk(WorldChunk* chunk, Vector3ic* coord) {
    assert(chunk != NULL);
    bool contains_pos = chunk_contains(chunk, coord);

    if (!contains_pos) {
        chunk = get_chunk_from_quad(chunk->parent, coord, NULL);
        if (chunk == NULL) return NULL;
    }

    Vector3i c_zero = chunk->zero_pos;
    return &(chunk->tiles[index(coord->x - c_zero.x, coord->y - c_zero.y, coord->z - c_zero.z)]);
}

int world_initialize_area(World* world, const BoundingBox area, int initial_flags) {
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
                    init_quadrant_leaf(quadrant, initial_flags);
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
    list_init(&new_world->fluids_to_update, sizeof(FluidUpdateData), WORLD_DEFAULT_ENTITY_LIST_CAPACITY);
    LOG_INFO_F("Created world with %d layers", new_world->depth);

    return new_world;
}

void free_quad(WorldQuadrant* quad) {
    if (quad == NULL) return;

    if (quad->height == 0) {
        // free leaf quadrant
        for (int i = 0; i < 8; ++i) {
            // free chunk
            WorldChunk* chunk = quad->leaves[i];
            for (int j = 0; j < CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_LENGTH; ++j) {
                // free tile
                list_free(&(chunk->tiles[j].entity_ptrs));
            }
            map_free(&chunk->fluids);
        }
        free(quad->leaves[0]); // chunks are allocated per 8

    } else {
        for (int i = 0; i < 8; ++i) {
            free_quad(quad->childs[i]);
        }
    }

    // quad data itself is allocated in world.quad_allocator
}

WorldChunk* world_get_chunk_from_chunk(WorldChunk* chunk, Vector3ic* coord) {
  if (chunk_contains(chunk, coord)) {
    return chunk;
  } else {
    return get_chunk_from_quad(chunk->parent, coord, NULL);
  }
}

void world_free(World* world) {
    free_quad(world->chunks);
    list_free(&world->entities_to_update);
    list_free(&world->fluids_to_update);
    allocator_sm_free(world->quad_allocator);
    free(world);
}

PURE List* world_get_entities_to_update(World* world) {
    return &world->entities_to_update;
}

Vector3ic* chunk_get_position(WorldChunk* chunk) {
    return &chunk->zero_pos;
}

PURE int to_int_inflate(float x) {
    const int tx = (int) x;
    return (x == tx) ? tx : (x < 0 ? tx - 1 : tx + 1);
}

WorldDirectionalIterator world_directional_iterator(
        const World* world, Vector3fc* focus, float width, float height, bool x_pos, bool y_pos, int z_min
) {
    Vector3ic focus_i = (Vector3i) {
            to_int_inflate(focus->x),
            to_int_inflate(focus->y),
            to_int_inflate(focus->z)
    };

    // half width and height
    const int width_h_i = to_int_inflate(width / 2);
    const int height_h_i = to_int_inflate(height / 2);
    assert(width_h_i > 0);
    assert(height_h_i > 0);

    const bool equi_directional = (x_pos == y_pos);
    // half-size parallel to x=y
    const int equ_h_size = equi_directional ? width_h_i : height_h_i;
    // half-size orthogonal to x=y (parallel to x=-y)
    const int diff_h_size = equi_directional ? height_h_i : width_h_i;

    const int x_add_y_min = focus_i.x + focus_i.y - 2 * equ_h_size;
    const int x_add_y_max = focus_i.x + focus_i.y + 2 * equ_h_size;
    const int x_sub_y_min = focus_i.x - focus_i.y - 2 * diff_h_size;
    const int x_sub_y_max = focus_i.x - focus_i.y + 2 * diff_h_size;

    const int x_add_y_begin = y_pos ? x_add_y_min : x_add_y_max; // a
    const int x_sub_y_begin = !y_pos ? x_sub_y_min : x_sub_y_max; // b

    // x + y = a & x - y = b ; x = b + y ; b + y + y = a ; 2y = a - b
    const int y = (x_add_y_begin - x_sub_y_begin) / 2;
    Vector3ic current = {x_sub_y_begin + y, y, focus_i.z};

    WorldQuadrant* quadrant = get_quad_under_quad(world->chunks, &focus_i, NULL);

    return (WorldDirectionalIterator) {
            world,
            quadrant,
            current,
            x_sub_y_min,
            x_sub_y_max,
            x_add_y_min,
            x_add_y_max,
            z_min,
            x_pos, y_pos
    };
}

WorldTileData world_directional_iterator_next(WorldDirectionalIterator* itr) {
    Vector3i position = itr->current;

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

    // update x
    const int xDir = itr->xp ? 1 : -1; // -watch_dir
    itr->current.x += xDir;

    // check bounds.
    int x_sub_y = itr->current.x - itr->current.y;
    int x_add_y = itr->current.x + itr->current.y;
    // Only two bounds will ever be met, but figuring out which two is more expensive than just testing all
    if (x_sub_y > itr->x_sub_y_max || x_sub_y < itr->x_sub_y_min || x_add_y > itr->x_add_y_max || x_add_y < itr->x_add_y_min) {
        // update y
        const int yDir = itr->yp ? 1 : -1;
        itr->current.y += yDir;

        // re-calculate the two limits of the x starting value
        const int x_add_y_begin = itr->yp ? itr->x_add_y_min : itr->x_add_y_max; // a
        const int x_sub_y_begin = !itr->yp ? itr->x_sub_y_min : itr->x_sub_y_max; // b
        const int add_lim = x_sub_y_begin + itr->current.y;
        const int sub_lim = x_add_y_begin - itr->current.y;
        // One of these limit x. Instead of calculating which one, we take the logic value regarding iteration direction
        itr->current.x = ((sub_lim < add_lim) == itr->xp) ? sub_lim : add_lim;

        // check whether the new start of (x, y) is out-of-bounds as well
        x_sub_y = itr->current.x - itr->current.y;
        x_add_y = itr->current.x + itr->current.y;
        if (x_sub_y > itr->x_sub_y_max || x_sub_y < itr->x_sub_y_min || x_add_y > itr->x_add_y_max || x_add_y < itr->x_add_y_min) {
            // then we need to go to the next z-level
            itr->current.z--; // z is always down

            itr->x_sub_y_min += xDir - yDir;
            itr->x_sub_y_max += xDir - yDir;
            itr->x_add_y_min += xDir + yDir;
            itr->x_add_y_max += xDir + yDir;

            // copied from iterator initializer
            itr->current.y = (x_add_y_begin - x_sub_y_begin) / 2;
            itr->current.x = x_sub_y_begin + itr->current.y;
        }

        // if the new (x, y) is still out of bounds, it would mean that width or height is 0 (which shouldn't happen)
    }

    return tile_data;
}

bool world_directional_iterator_has_next(WorldDirectionalIterator* itr) {
    return itr->current.z >= itr->z_min;
}

WorldTile* world_get_tile(World* world, Vector3ic* coord) {
    return get_tile_under_quad(world->chunks, coord);
}

WorldChunk* world_get_chunk(World* world, Vector3ic* coord) {
    return get_chunk_under_quad(world->chunks, coord, NULL);
}

void world_tile_init(WorldTile* base_tile, char flags) {
    base_tile->flags = flags;
    list_init(&base_tile->entity_ptrs, sizeof(void*), 0);
}


void world_tile_add_entity(Entity* entity, WorldChunk* chunk, WorldTile* tile, Vector3ic* coord) {
    entity->chunk = chunk;
    entity->position = *coord;

    if (entity->class->flags & ENTITY_FLAG_BLOCKING) {
        tile->flags |= TILE_FLAG_BLOCKING;
    }

    list_add(&tile->entity_ptrs, &entity);
}

List* world_get_fluids_to_update(World* world) {
    return &world->fluids_to_update;
}

void world_tile_remove_entity(WorldTile* tile, Entity* entity) {
    list_delete_value(&tile->entity_ptrs, &entity);

    if (entity->class->flags & ENTITY_FLAG_BLOCKING) {
        bool blocking = false;
        for (int i = 0; i < list_size(&tile->entity_ptrs); ++i) {
            Entity** ety = list_get(&tile->entity_ptrs, i);
            if ((*ety)->class->flags & ENTITY_FLAG_BLOCKING) {
                blocking = true;
                break;
            }
        }
        if (!blocking) {
            tile->flags &= ~ENTITY_FLAG_BLOCKING;
        }
    }
}

FluidFlow* tile_get_fluid(WorldTile* tile, WorldChunk* chunk) {
    FluidFlow* fluid = map_get(&chunk->fluids, tile->index);
    if (!fluid) {
        FluidFlow new_fluid = (FluidFlow) {};
        fluid = map_insert(&chunk->fluids, tile->index, &new_fluid);
    }
    return fluid;
}

FluidFlow* world_get_fluid(World* world, Vector3ic* coord) {
    WorldChunk* chunk = get_chunk_from_quad(world->chunks, coord, NULL);
    if (chunk == NULL) return NULL;

    WorldTile* tile = world_get_tile_from_chunk(chunk, coord);
    return tile_get_fluid(tile, chunk);
}

Vector3i world_get_tile_coord(WorldTile* tile, WorldChunk* parent) {
    Vector3i pos = parent->zero_pos;
    int ix = tile->index % (CHUNK_LENGTH * CHUNK_LENGTH);
    int iy = (tile->index % (CHUNK_LENGTH)) / CHUNK_LENGTH;
    int iz = tile->index / (CHUNK_LENGTH * CHUNK_LENGTH);
    return (Vector3i) {ix, iy, iz};
}
