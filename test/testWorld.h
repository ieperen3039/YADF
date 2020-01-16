//
// Created by s152717 on 9-1-2020.
//

#ifndef YADF_TESTWORLD_H
#define YADF_TESTWORLD_H

#include "CuTest.h"

#define YADF_STATIC

#include "../src/YADFEngine/World/World.c"

#include <limits.h>

#define AREA_SIZE (CHUNK_LENGTH + 4)
#define AREA_MIN_2 (AREA_SIZE - 2)
#define AREA_PLUS_2 (AREA_SIZE + 2)

void test_world_new(CuTest* tc) {
    printf(
            "\nChunks are (%d x %d x %d) in size, with %d tiles per chunk, and a data size of %zu KB\n",
            CHUNK_LENGTH, CHUNK_LENGTH, CHUNK_LENGTH, CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_LENGTH,
            sizeof(WorldChunk) / 1000
    );

    World* world = world_new(INT_MAX);
    CuAssertPtrNotNull(tc, world);
    unsigned long long n_ch = 1 << world->depth;

    printf(
            "Maximum world size is %llu tiles wide, containing %llu chunks per z-level, with a tree depth of %d\n",
            n_ch * CHUNK_LENGTH, n_ch * n_ch, world->depth
    );
}

#define CuAssertVec3iEquals(tc, ex, ac) CuAssertVec3iEquals_LineMsg((tc),__FILE__,__LINE__,NULL,(ex),(ac))

void CuAssertVec3iEquals_LineMsg(CuTest* tc, const char* file, int line, const char* message, Vector3i expected,
                                 Vector3i actual) {
    char buf[STRING_MAX];
    if (expected.x == actual.x && expected.y == actual.y && expected.z == actual.z) return;
    sprintf(buf,
            "expected (%d, %d, %d) but was (%d, %d, %d)",
            expected.x, expected.y, expected.z, actual.x, actual.y, actual.z
    );
    CuFail_Line(tc, file, line, message, buf);
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

void test_world_directional_itr_small_pos(CuTest* tc) {
    World* world = world_new(100);
    WorldTile base_tile = {LIST_EMPTY, TILE_FLAG_VISIBLE};
    BoundingBox area = (BoundingBox) {-10, -10, -10, 10, 10, 10};
    world_initialize_area(world, area, base_tile);

    Vector3f two = {2, 2, 2};
    WorldDirectionalIterator itr = world_directional_iterator(world, &VECTOR_ZERO, &two, true, true);

    { // once
        CuAssertTrue(tc, world_directional_iterator_has_next(&itr));
        WorldTileData tile = world_directional_iterator_next(&itr);
        CuAssertVec3iEquals(tc, ((Vector3i) {-1, -1, 0}), tile.coord);
    }

    for (int i = 0; i < 3; ++i) {
        CuAssertTrue(tc, world_directional_iterator_has_next(&itr));
        WorldTileData tile = world_directional_iterator_next(&itr);
        // two negative, one zero (if only z was also positive)
        CuAssertIntEquals(tc, -2, tile.coord.x + tile.coord.y + (1 - tile.coord.z));
    }

    for (int i = 0; i < 3; ++i) {
        CuAssertTrue(tc, world_directional_iterator_has_next(&itr));
        WorldTileData tile = world_directional_iterator_next(&itr);
        // one negative, two zero (if only z was also positive)
        CuAssertIntEquals(tc, -1, tile.coord.x + tile.coord.y + (1 - tile.coord.z));
    }

    { // once
        CuAssertTrue(tc, world_directional_iterator_has_next(&itr));
        WorldTileData tile = world_directional_iterator_next(&itr);
        CuAssertVec3iEquals(tc, ((Vector3i) {0, 0, -1}), tile.coord);
    }

    CuAssertTrue(tc, !world_directional_iterator_has_next(&itr));
}

// basically a copy of test_world_directional_itr_small_pos
void test_world_directional_itr_small_neg(CuTest* tc) {
    World* world = world_new(100);
    WorldTile base_tile = {LIST_EMPTY, TILE_FLAG_VISIBLE};
    BoundingBox area = (BoundingBox) {-10, -10, -10, 10, 10, 10};
    world_initialize_area(world, area, base_tile);

    Vector3f two = {2, 2, 2};
    WorldDirectionalIterator itr = world_directional_iterator(world, &VECTOR_ZERO, &two, false, false);

    { // once
        CuAssertTrue(tc, world_directional_iterator_has_next(&itr));
        WorldTileData tile = world_directional_iterator_next(&itr);
        CuAssertVec3iEquals(tc, ((Vector3i) {0, 0, 0}), tile.coord);
    }

    for (int i = 0; i < 3; ++i) {
        CuAssertTrue(tc, world_directional_iterator_has_next(&itr));
        WorldTileData tile = world_directional_iterator_next(&itr);
        // one negative, two zero
        CuAssertIntEquals(tc, -1, tile.coord.x + tile.coord.y + tile.coord.z);
    }

    for (int i = 0; i < 3; ++i) {
        CuAssertTrue(tc, world_directional_iterator_has_next(&itr));
        WorldTileData tile = world_directional_iterator_next(&itr);
        // two negative, one zero
        CuAssertIntEquals(tc, -2, tile.coord.x + tile.coord.y + tile.coord.z);
    }

    { // once
        CuAssertTrue(tc, world_directional_iterator_has_next(&itr));
        WorldTileData tile = world_directional_iterator_next(&itr);
        CuAssertVec3iEquals(tc, ((Vector3i) {-1, -1, -1}), tile.coord);
    }

    CuAssertTrue(tc, !world_directional_iterator_has_next(&itr));
}

void test_world_directional_itr_small_cross(CuTest* tc) {
    World* world = world_new(100);
    WorldTile base_tile = {LIST_EMPTY, TILE_FLAG_VISIBLE};
    BoundingBox area = (BoundingBox) {-10, -10, -10, 10, 10, 10};
    world_initialize_area(world, area, base_tile);

    Vector3f two = {2, 2, 2};
    WorldDirectionalIterator itr1 = world_directional_iterator(world, &VECTOR_ZERO, &two, false, true);
    { // once
        CuAssertTrue(tc, world_directional_iterator_has_next(&itr1));
        WorldTileData tile = world_directional_iterator_next(&itr1);
        CuAssertVec3iEquals(tc, ((Vector3i) {0, -1, 0}), tile.coord);
    }
    for (int i = 0; i < 6; ++i) {
        CuAssertTrue(tc, world_directional_iterator_has_next(&itr1));
        world_directional_iterator_next(&itr1);
    }
    { // once
        CuAssertTrue(tc, world_directional_iterator_has_next(&itr1));
        WorldTileData tile = world_directional_iterator_next(&itr1);
        CuAssertVec3iEquals(tc, ((Vector3i) {-1, 0, -1}), tile.coord);
    }

    WorldDirectionalIterator itr2 = world_directional_iterator(world, &VECTOR_ZERO, &two, true, false);
    { // once
        CuAssertTrue(tc, world_directional_iterator_has_next(&itr2));
        WorldTileData tile = world_directional_iterator_next(&itr2);
        CuAssertVec3iEquals(tc, ((Vector3i) {-1, 0, 0}), tile.coord);
    }
    for (int i = 0; i < 6; ++i) {
        CuAssertTrue(tc, world_directional_iterator_has_next(&itr2));
        world_directional_iterator_next(&itr2);
    }
    { // once
        CuAssertTrue(tc, world_directional_iterator_has_next(&itr2));
        WorldTileData tile = world_directional_iterator_next(&itr2);
        CuAssertVec3iEquals(tc, ((Vector3i) {0, -1, -1}), tile.coord);
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
    SUITE_ADD_TEST(suite, test_world_directional_itr_large);

    return suite;
}

#endif //YADF_TESTWORLD_H
