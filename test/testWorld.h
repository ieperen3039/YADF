//
// Created by s152717 on 9-1-2020.
//

#ifndef YADF_TESTWORLD_H
#define YADF_TESTWORLD_H

#include "CuTest.h"
#include "CuExtra.h"

#define YADF_STATIC

#include "../src/YADFEngine/World/World.c"

#include <limits.h>

#define AREA_SIZE (CHUNK_LENGTH + 4)
#define AREA_MIN_2 (AREA_SIZE - 2)
#define AREA_PLUS_2 (AREA_SIZE + 2)

void test_world_new(CuTest* tc) {
    printf(
            "\nChunks are (%d x %d x %d) in size, with %d tiles per chunk, and a data size of %lu KB\n",
            CHUNK_LENGTH, CHUNK_LENGTH, CHUNK_LENGTH, CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_LENGTH,
            sizeof(WorldChunk) / 1000L
    );

    World* world = world_new(INT_MAX);
    CuAssertPtrNotNull(tc, world);
    unsigned long long n_ch = 1 << world->depth;

    printf(
            "Maximum world size is %llu tiles wide, containing %llu chunks per z-level, with a tree depth of %d\n",
            n_ch * CHUNK_LENGTH, n_ch * n_ch, world->depth
    );
}

void test_world_tile_data(CuTest* tc) {
    World* world = world_new(100);
    WorldTile base_tile = {LIST_EMPTY, TILE_FLAG_VISIBLE};
    BoundingBox area = (BoundingBox) {-AREA_SIZE, -AREA_SIZE, -AREA_SIZE, AREA_SIZE, AREA_SIZE, AREA_SIZE};
    world_initialize_area(world, area, base_tile);

    Vector3i zero = (Vector3i) {0, 0, 0};
    WorldTile* tile1 = world_get_tile(world, &zero);
    CuAssertTrue(tc, tile1->flags & TILE_FLAG_VISIBLE);
    tile1->flags |= TILE_FLAG_OPAQUE;

    Vector3i pos2 = (Vector3i) {AREA_MIN_2, -AREA_MIN_2, -AREA_MIN_2};
    WorldTile* tile2 = world_get_tile(world, &zero);
    CuAssertTrue(tc, tile2->flags & TILE_FLAG_VISIBLE);
    tile2->flags |= TILE_FLAG_OPAQUE;

    WorldTile* tile3 = world_get_tile(world, &zero);
    CuAssertPtrEquals(tc, tile3, tile1); // must point to the exact same tile
    CuAssertTrue(tc, tile2->flags & TILE_FLAG_OPAQUE);
}

void test_world_iterator(CuTest* tc) {
    World* world = world_new(100);
    WorldTile base_tile = {LIST_EMPTY, TILE_FLAG_VISIBLE};
    BoundingBox area = (BoundingBox) {-AREA_SIZE, -AREA_SIZE, -AREA_SIZE, AREA_SIZE, AREA_SIZE, AREA_SIZE};
    world_initialize_area(world, area, base_tile);

    Vector3i positions[] = {
            {1,           2,  3},
            {0,           0,  0},
            {CHUNK_LENGTH, CHUNK_LENGTH, CHUNK_LENGTH},
            {-1,          0,  0},
            {-1,          -1, 0},
            {1,           -1, 1},
            {AREA_MIN_2,  0,  0},
            {AREA_MIN_2,  0,  -1},
            {-AREA_MIN_2, -AREA_MIN_2,   AREA_MIN_2}
    };

    // mark specific flags
    size_t num_positions = sizeof(positions) / sizeof(Vector3i);
    for (int i = 0; i < num_positions; i++) {
        WorldTile* tile = world_get_tile(world, &positions[i]);
        tile->flags |= TILE_FLAG_OPAQUE;
    }

    WorldChunkIterator itr = world_get_chunk_iterator(world, area);
    while (world_chunk_iterator_has_next(&itr)) {
        WorldChunkData chunk = world_chunk_iterator_next(&itr);
        CuAssertPtrNotNull(tc, chunk.elt);
        CuAssertVec3iEquals(tc, chunk.elt->zero_pos, chunk.coord);

        WorldTileIterator itr2 = chunk_get_tile_iterator(chunk.elt);
        while (chunk_tile_iterator_has_next(&itr2)) {
            WorldTileData tile = chunk_tile_iterator_next(&itr2);
            WorldTile* elt = tile.elt;

            CuAssert(tc, "Tile has been returned twice", !(elt->flags & TILE_FLAG_DISCOVERED));

            // check specific tiles
            bool has_found = false;
            for (int i = 0; i < num_positions; i++) {
                if (
                        positions[i].x == tile.coord.x &&
                        positions[i].y == tile.coord.y &&
                        positions[i].z == tile.coord.z
                        ) {
                    CuAssertIntEquals(tc, TILE_FLAG_VISIBLE | TILE_FLAG_OPAQUE, elt->flags);
                    has_found = true;
                }
            }

            if (!has_found) {
                CuAssertIntEquals(tc, TILE_FLAG_VISIBLE, elt->flags);
            }

            // mark tile
            elt->flags |= TILE_FLAG_DISCOVERED;
        }
    }

    // check that all tiles have been returned
    for (int ix = area.xMin; ix <= area.xMax; ++ix) {
        for (int iy = area.yMin; iy <= area.yMax; ++iy) {
            for (int iz = 0; iz <= area.zMax; ++iz) {
                Vector3i coord = {ix, iy, iz};
                WorldTile* tile = world_get_tile(world, &coord);
                CuAssertTrue(tc, tile->flags & TILE_FLAG_DISCOVERED);
            }
        }
    }
}

int test_direction(CuTest* tc, WorldDirectionalIterator* itr, const int x_dir, const int y_dir) {
    const int z_dir = -1;

    List seen;
    list_init(&seen, sizeof(Vector3i), 5 * 5 * 5);

    while (world_directional_iterator_has_next(itr)) {
        WorldTileData tile = world_directional_iterator_next(itr);
        Vector3i tile_coord = tile.coord;

        for (int i = 0; i < list_get_size(&seen); ++i) {
            Vector3i* other = list_get(&seen, i);

            CuAssert(tc, "Tile has been returned before", !vectori_equals(other, &tile_coord));

            Vector3i delta = {tile_coord.x - other->x, tile_coord.y - other->y, tile_coord.z - other->z};
            delta.x *= x_dir;
            delta.y *= y_dir;
            delta.z *= z_dir;
            // from here, delta is in the direction

            bool in_front = (delta.x == delta.y && delta.y == delta.z && delta.x < 0);
            CuAssert(tc, "Tile is right in front of an earlier tile", !in_front);

            bool overlap_yz = (delta.x - 1 == delta.y && delta.y == delta.z && delta.y < 0);
            bool overlap_xz = (delta.x == delta.y - 1 && delta.x == delta.z && delta.x < 0);
            bool overlap_xy = (delta.x == delta.y && delta.x == delta.z - 1 && delta.x < 0);
            CuAssert(tc, "Tile overlaps an earlier tile", !(overlap_yz || overlap_xz || overlap_xy));
        }

        list_add(&seen, &tile_coord);
    }
    
    int nr_seen = list_get_size(&seen);
    list_free(&seen);
    return nr_seen;
}

void test_world_directional_itr_small(CuTest* tc) {
    World* world = world_new(100);
    WorldTile base_tile = {LIST_EMPTY, TILE_FLAG_VISIBLE};
    BoundingBox area = (BoundingBox) {-10, -10, -10, 10, 10, 10};
    world_initialize_area(world, area, base_tile);

    { // base case
        WorldDirectionalIterator itr = world_directional_iterator(world, &VECTOR_ZERO, 2, 2, false, false, -2);
        int i = test_direction(tc, &itr, -1, -1);
        LOG_INFO_F("Base case 2x2x2 returned %d tiles", i);
        CuAssert(tc, "Not enough tiles returned", i > 8);
    }
    { // 3x3x3 block, direction 1/4
        WorldDirectionalIterator itr = world_directional_iterator(world, &VECTOR_ZERO, 3, 3, false, false, -3);
        int i = test_direction(tc, &itr, -1, -1);
        CuAssert(tc, "Not enough tiles returned", i > 27);
    }
    { // 3x3x3 block, direction 2/4
        WorldDirectionalIterator itr = world_directional_iterator(world, &VECTOR_ZERO, 3, 3, true, true, -3);
        int i = test_direction(tc, &itr, 1, 1);
        CuAssert(tc, "Not enough tiles returned", i > 27);
    }
    { // 3x3x3 block, direction 3/4
        WorldDirectionalIterator itr = world_directional_iterator(world, &VECTOR_ZERO, 3, 3, true, false, -3);
        int i = test_direction(tc, &itr, 1, -1);
        CuAssert(tc, "Not enough tiles returned", i > 27);
    }
    { // 3x3x3 block, direction 4/4
        WorldDirectionalIterator itr = world_directional_iterator(world, &VECTOR_ZERO, 3, 3, false, true, -3);
        int i = test_direction(tc, &itr, -1, 1);
        CuAssert(tc, "Not enough tiles returned", i > 27);
    }
    { // large block, non-zero focus
        Vector3fc focus = {3, 4, 5};
        WorldDirectionalIterator itr = world_directional_iterator(world, &focus, 10, 20, false, false, 0);
        int i = test_direction(tc, &itr, -1, -1);
        CuAssert(tc, "Not enough tiles returned", i > 100);
    }
    { // large block, negative non-zero focus, mixed direction
        Vector3fc focus = {-3, -4, -5};
        WorldDirectionalIterator itr = world_directional_iterator(world, &focus, 10, 20, false, true, -20);
        int i = test_direction(tc, &itr, -1, 1);
        CuAssert(tc, "Not enough tiles returned", i > 100);
    }
}

void test_world_directional_itr_large(CuTest* tc) {
    World* world = world_new(200);
    WorldTile base_tile = {LIST_EMPTY, TILE_FLAG_VISIBLE};
    BoundingBox area = (BoundingBox) {-AREA_SIZE, -AREA_SIZE, -AREA_SIZE, AREA_SIZE, AREA_SIZE, AREA_SIZE};
    world_initialize_area(world, area, base_tile);

    Vector3i positions[] = {
            {1,           2,  3},
            {0,           0,  0},
            {CHUNK_LENGTH, CHUNK_LENGTH, CHUNK_LENGTH},
            {-1,          0,  0},
            {-1,          -1, 0},
            {1,           -1, 1},
            {AREA_MIN_2,  0,  0},
            {AREA_MIN_2,  0,  -1},
            {-AREA_MIN_2, -AREA_MIN_2,   AREA_MIN_2}
    };

    // mark specific flags
    size_t num_positions = sizeof(positions) / sizeof(Vector3i);
    for (int i = 0; i < num_positions; i++) {
        WorldTile* tile = world_get_tile(world, &positions[i]);
        tile->flags |= TILE_FLAG_OPAQUE;
    }
}

CuSuite* world_suite(void) {
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_world_new);
    SUITE_ADD_TEST(suite, test_world_tile_data);
    SUITE_ADD_TEST(suite, test_world_iterator);
    SUITE_ADD_TEST(suite, test_world_directional_itr_small);
    SUITE_ADD_TEST(suite, test_world_directional_itr_large);

    return suite;
}

#endif //YADF_TESTWORLD_H
