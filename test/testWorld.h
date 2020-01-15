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
            CHUNK_LENGTH, CHUNK_LENGTH, CHUNK_LENGTH, CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_LENGTH, sizeof(WorldChunk) / 1000
    );

    World* world = world_new(INT_MAX);
    CuAssertPtrNotNull(tc, world);
    unsigned long long n_ch = 1 << world->depth;

    printf("Maximum world size is %llu tiles wide, containing %llu chunks per z-level, with a tree depth of %d\n",
            n_ch * CHUNK_LENGTH, n_ch * n_ch, world->depth);
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
        world_get_tile(world, &positions[i])->flags |= TILE_FLAG_OPAQUE;
    }

    WorldChunkIterator itr = world_get_chunk_iterator(world, area);
    while (world_chunk_iterator_has_next(&itr)) {
        WorldChunkData chunk = world_chunk_iterator_next(&itr);
        CuAssertIntEquals(tc, chunk.elt->zero_pos.x, chunk.coord.x);
        CuAssertIntEquals(tc, chunk.elt->zero_pos.y, chunk.coord.y);
        CuAssertIntEquals(tc, chunk.elt->zero_pos.z, chunk.coord.z);
        LOG_INFO_F("Chunk : (%d, %d, %d)", chunk.coord.x, chunk.coord.y, chunk.coord.z);

        WorldTileIterator itr2 = chunk_get_tile_iterator(chunk.elt);
        while (chunk_tile_iterator_has_next(&itr2)) {
            WorldTileData tile = chunk_tile_iterator_next(&itr2);

            CuAssert(tc, "Tile has been returned twice", !(tile.elt->flags & TILE_FLAG_DISCOVERED));

            // check specific tiles
            bool has_found = false;
            for (int i = 0; i < num_positions; i++) {
                if (
                        positions[i].x == tile.coord.x &&
                        positions[i].y == tile.coord.y &&
                        positions[i].z == tile.coord.z
                        ) {
                    CuAssertIntEquals(tc, TILE_FLAG_VISIBLE | TILE_FLAG_OPAQUE, tile.elt->flags);
                    has_found = true;
                }
            }

            if (!has_found) {
                CuAssertIntEquals(tc, TILE_FLAG_VISIBLE, tile.elt->flags);
            }

            // mark tile
            tile.elt->flags |= TILE_FLAG_DISCOVERED;
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

CuSuite* world_suite(void) {
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_world_new);
    SUITE_ADD_TEST(suite, test_world_tile_data);
    SUITE_ADD_TEST(suite, test_world_iterator);

    return suite;
}

#endif //YADF_TESTWORLD_H
