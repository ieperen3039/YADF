//
// Created by s152717 on 28-12-2019.
//

#ifndef YADF_WORLDGENERATOR_H
#define YADF_WORLDGENERATOR_H

#include "../Entities/Entity.h"
#include "../External/open-simplex-noise.h"
#include "../Entities/EntityImplStatic.h"

static void generator_generate_default(World* world, BoundingBox area) {
    WorldTile base_tile;
    world_tile_init(&base_tile, TILE_FLAG_VISIBLE | TILE_FLAG_DISCOVERED);

    int new_chunks = world_initialize_area(world, area, base_tile);

    struct osn_context* noise;
    open_simplex_noise(0, &noise);
    double noise_scale = 10.0;

    // set ground to slate blocks

    WorldChunkIterator chunk_itr = world_get_chunk_iterator(world, area);
    while (world_chunk_iterator_has_next(&chunk_itr)) {
        WorldChunkData chunk = world_chunk_iterator_next(&chunk_itr);
        assert(chunk.elt != NULL);

        WorldTileIterator tile_itr = chunk_get_tile_iterator(chunk.elt);
        while (chunk_tile_iterator_has_next(&tile_itr)) {
            WorldTileData tile = chunk_tile_iterator_next(&tile_itr);

            double noise_value = open_simplex_noise3(
                    noise, tile.coord.x / noise_scale, tile.coord.y / noise_scale, tile.coord.z / noise_scale
            );

            if (noise_value > 0.5f) {
                Material mat[] = {SLATE};
                Entity* rock = entity_new(NATURAL_WALL, NULL, mat);
                world_tile_add_entity(tile, rock, chunk.elt);
            }
        }
    }

    LOG_INFO_F("Initialized %d new chunks", new_chunks);
}

#endif //YADF_WORLDGENERATOR_H
