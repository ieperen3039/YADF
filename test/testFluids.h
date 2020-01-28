//
// Created by ieperen3039 on 23-01-20.
//

#ifndef YADF_TESTFLUIDS_H
#define YADF_TESTFLUIDS_H

#include "CuTest.h"

#define YADF_STATIC

#include <WorldAPI.h>
#include <EntityImpl.h>
#include "../src/YADFEngine/World/World.h"
#include "../src/YADFEngine/Entities/Fluids.c"
#include "../src/YADFEngine/Entities/EntityWrite.h"
#include "../src/YADFEngine/Entities/EntityClass.h"

/// world from z = -10 to -1
PURE World* create_flat_world() {
  World* world = world_new(50);
  const BoundingBox area = (BoundingBox) {-10, -10, -10, 10, 10, 10};
  world_initialize_area(world, area, TILE_FLAG_VISIBLE);

  entity_class_init();
  struct EntityImplNaturalWall data = {SLATE};

  BoundingBox ground = (BoundingBox) {-10, -10, -10, 10, 10, 0};
  WorldChunkIterator chunk_itr = world_get_chunk_iterator(world, ground);
  while (world_chunk_iterator_has_next(&chunk_itr)) {
    WorldChunkData chunk = world_chunk_iterator_next(&chunk_itr);

    WorldTileIterator tile_itr = chunk_get_tile_iterator(chunk.elt);
    while (chunk_tile_iterator_has_next(&tile_itr)) {
      WorldTileData tile = chunk_tile_iterator_next(&tile_itr);
      if (tile.coord.z >= 0) continue;

      Entity* rock = entity_new(NATURAL_WALL, &data);
      world_tile_add_entity(rock, chunk.elt, tile.elt, &tile.coord);
    }
  }

  return world;
}

void test_fluid_init(CuTest* tc) {
  World* world = create_flat_world();

  // check empty
  for (int x = -5; x < 5; x++) {
    for (int y = -5; y < 5; y++) {
      for (int z = -5; z < 5; z++) {
        Vector3i coord = (Vector3i) {x, y, z};
        // there are no fluids on this world, everything should be dry
        FluidFlow* fluid = world_get_fluid(world, &coord);
        CuAssertPtrNotNull(tc, fluid);

        CuAssertIntEquals(tc, 0, fluid->amount_total);
        CuAssertIntEquals(tc, 0, fluid->amount_total_prev);
        CuAssertIntEquals(tc, 0, fluid->flow_out_total);

        for (int i = 0; i < FluidTypeSize; ++i) {
          CuAssertIntEquals(tc, 0, fluid->amount[i]);
          CuAssertIntEquals(tc, 0, fluid->amount_prev[i]);
        }
      }
    }
  }

  Vector3i placement = (Vector3i) {1, -1, 0};
  fluid_spawn(world, &placement, WATER, 42);

  // check data corruption
  for (int x = -5; x < 5; x++) {
    for (int y = -5; y < 5; y++) {
      for (int z = -5; z < 5; z++) {
        Vector3i coord = (Vector3i) {x, y, z};
        FluidFlow* fluid = world_get_fluid(world, &coord);
        CuAssertPtrNotNull(tc, fluid);

        if (x == placement.x && y == placement.y && z == placement.z){
          CuAssertIntEquals(tc, 42, fluid->amount_total);
          CuAssertIntEquals(tc, 0, fluid->amount_total_prev);
          CuAssertIntEquals(tc, 0, fluid->flow_out_total);

          for (int i = 0; i < FluidTypeSize; ++i) {
            if (i == WATER){
              CuAssertIntEquals(tc, 42, fluid->amount[i]);
              CuAssertIntEquals(tc, 0, fluid->amount_prev[i]);
            } else {
              CuAssertIntEquals(tc, 0, fluid->amount[i]);
              CuAssertIntEquals(tc, 0, fluid->amount_prev[i]);
            }
          }
        } else {
          CuAssertIntEquals(tc, 0, fluid->amount_total);
          CuAssertIntEquals(tc, 0, fluid->amount_total_prev);
          CuAssertIntEquals(tc, 0, fluid->flow_out_total);

          for (int i = 0; i < FluidTypeSize; ++i) {
            CuAssertIntEquals(tc, 0, fluid->amount[i]);
            CuAssertIntEquals(tc, 0, fluid->amount_prev[i]);
          }
        }
      }
    }
  }

  world_free(world);
}

void print_fluid_levels(CuTest* tc, World* world, int z_level) {
  LOG_INFO("Fluid levels:");
  for (int x = -5; x <= 5; ++x) {
    for (int y = -5; y <= 5; ++y) {
      Vector3i coord = (Vector3i) {x, y, z_level};
      WorldTile* tile = world_get_tile(world, &coord);

      if (tile->flags & TILE_FLAG_BLOCKING){
        printf("XX ");

      } else {
        FluidFlow* fluid = world_get_fluid(world, &coord);
        CuAssertPtrNotNull(tc, fluid);
        printf("%2d ", (int) (fluid->amount_total * 99.0 / FLUID_AMOUNT_MAX));
      }
    }
    printf("\n");
  }
}

void test_fluid_fill_area(CuTest* tc) {
  World* world = create_flat_world();

  // create a wall
  struct EntityImplNaturalWall data = {SLATE};
  for (int x = -5; x <= 5; ++x) {
    Vector3i coord = (Vector3i) {x, -5, 0};
    WorldChunk* chunk = world_get_chunk(world, &coord);
    WorldTile* tile = world_get_tile_from_chunk(chunk, &coord);
    world_tile_add_entity(entity_new(NATURAL_WALL, &data), chunk, tile, &coord);
  }
  for (int x = -5; x <= 5; ++x) {
    Vector3i coord = (Vector3i) {x, 5, 0};
    WorldChunk* chunk = world_get_chunk(world, &coord);
    WorldTile* tile = world_get_tile_from_chunk(chunk, &coord);
    world_tile_add_entity(entity_new(NATURAL_WALL, &data), chunk, tile, &coord);
  }
  for (int y = -5; y <= 5; ++y) {
    Vector3i coord = (Vector3i) {-5, y, 0};
    WorldChunk* chunk = world_get_chunk(world, &coord);
    WorldTile* tile = world_get_tile_from_chunk(chunk, &coord);
    world_tile_add_entity(entity_new(NATURAL_WALL, &data), chunk, tile, &coord);
  }
  for (int y = -5; y <= 5; ++y) {
    Vector3i coord = (Vector3i) {5, y, 0};
    WorldChunk* chunk = world_get_chunk(world, &coord);
    WorldTile* tile = world_get_tile_from_chunk(chunk, &coord);
    world_tile_add_entity(entity_new(NATURAL_WALL, &data), chunk, tile, &coord);
  }

  { // place fluid
    Vector3i coord = (Vector3i) {0, 0, 0};
    fluid_spawn(world, &coord, WATER, FLUID_AMOUNT_MAX);
  }

  const int FLUID_ITERATIONS = 0;

  for (int i = 0; i < FLUID_ITERATIONS; ++i) {
    print_fluid_levels(tc, world, 0);
    List* fluids = world_get_fluids_to_update(world);

    for (int j = 0; j < list_size(fluids); ++j) {
      FluidUpdateData* fd = list_get(fluids, j);
      fluid_flow_update(fd->element, fd->coord, fd->chunk);
    }

    for (int j = 0; j < list_size(fluids); ++j) {
      FluidUpdateData* fd = list_get(fluids, j);
      fluid_amount_update(fd->element, fd->coord, fd->chunk);
    }
  }

  // print fluid situation
  print_fluid_levels(tc, world, 0);
  world_free(world);
}

CuSuite* fluid_suite(void) {
  CuSuite* suite = CuSuiteNew();

  SUITE_ADD_TEST(suite, test_fluid_init);
  SUITE_ADD_TEST(suite, test_fluid_fill_area);

  return suite;
}

#endif //YADF_TESTFLUIDS_H
