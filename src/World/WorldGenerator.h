//
// Created by s152717 on 28-12-2019.
//

#ifndef YADF_WORLDGENERATOR_H
#define YADF_WORLDGENERATOR_H

#include "../Entities/Entity.h"
#include "../Entities/RockEntities.h"

static void generator_generate_default(World* world, BoundingBox area) {
    WorldTile base_tile;
    world_tile_init(&base_tile, TILE_FLAG_VISIBLE | TILE_FLAG_DISCOVERED);

    int new_chunks = world_initialize_area(world, area, base_tile);
    LOG_INFO_F("Initialized %d new chunks", new_chunks);

    // set ground to slate blocks
    BoundingBox area_ground = area;
    area_ground.zMax = 0;

    WorldChunkIterator chunk_itr = world_get_chunk_iterator(world, area_ground);
    while (world_chunk_iterator_has_next(&chunk_itr)) {
        WorldChunkData chunk = world_chunk_iterator_next(&chunk_itr);
        assert(chunk.elt != NULL);

        WorldTileIterator tile_itr = chunk_get_tile_iterator(chunk.elt);
        while (chunk_tile_iterator_has_next(&tile_itr)) {
            WorldTileData tile = chunk_tile_iterator_next(&tile_itr);

            Entity* rock = slate_new(NULL);
            world_tile_add_entity(tile, rock, chunk.elt);
        }
    }
}

#endif //YADF_WORLDGENERATOR_H
