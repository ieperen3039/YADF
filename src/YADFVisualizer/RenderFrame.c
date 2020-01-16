//
// Created by s152717 on 5-1-2020.
//

#include "RenderFrame.h"

#include <EntityImpl.h>

static inline void draw_entity(Shader* shader, const Entity* entity, const Color4f* material_map, const Sprite** entity_sprites) {
    switch (entity->class->type) {
        case NATURAL_WALL: {
            struct EntityImplNaturalWall* data = entity->entity_data;
            Color4f color = material_map[data->material];
            shader_set_color(shader, 0, color);
            break;
        }
        case VEGETATION_PATCH:
        case CREATURE:
            break;
    }

    sprite_draw(entity_sprites[entity->class->type], shader);
}

void render_frame(Shader* shader, World* world, const Vector3f* eye, const Color4f* material_map,
                  const Sprite** entity_sprites, int window_width, int window_height) {
    shader_bind(shader, eye, window_width, window_height);

    // TODO voxel culling
    BoundingBox box = {eye->x - 100, eye->y - 100, eye->z - 100, eye->x + 100, eye->y + 100, eye->z};

    WorldChunkIterator chunk_itr = world_get_chunk_iterator(world, box);
    while (world_chunk_iterator_has_next(&chunk_itr)) {
        WorldChunkData chunk = world_chunk_iterator_next(&chunk_itr);

        if (chunk.elt == NULL) {
            shader_set_tile_position(shader, chunk.coord);
            // TODO draw black?
            continue;
        }

        WorldTileIterator tile_itr = chunk_get_tile_iterator(chunk.elt);
        while (chunk_tile_iterator_has_next(&tile_itr)) {
            WorldTileData tile = chunk_tile_iterator_next(&tile_itr);

            shader_set_tile_position(shader, tile.coord);

            if (tile.coord.z != -15) continue;

            if (tile.elt->flags & (TILE_FLAG_DISCOVERED)) {
                // render all entities
                ListIterator ety_itr = list_iterator(&tile.elt->entity_ptrs);
                while (list_iterator_has_next(&ety_itr)) {
                    Entity** ety_pp = list_iterator_next(&ety_itr);
                    draw_entity(shader, *ety_pp, material_map, entity_sprites);
                }

            } else {
                // TODO render black cube
            }
        }
    }

    shader_unbind(shader);
}
