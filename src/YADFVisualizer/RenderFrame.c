//
// Created by s152717 on 5-1-2020.
//

#include "RenderFrame.h"

#include <EntityImpl.h>

#define VIEW_DEPTH 10

static inline void draw_entity(
        Shader* shader, const Entity* entity, const Color4f* material_map, const Sprite** entity_sprites
) {
    switch (entity->class->type) {
        case NATURAL_WALL: {
            struct EntityImplNaturalWall* data = entity->entity_data;
            Color4f color = material_map[data->material];
            shader_set_color(shader, 0, color);
            break;
        }
        case VEGETATION_PATCH:
        case CREATURE:break;
    }

    sprite_draw(entity_sprites[entity->class->type], shader);
}

void render_frame(
        Shader* shader, World* world, const Vector3f* eye, const Color4f* material_map,
        const Sprite** entity_sprites, int window_width, int window_height
) {
    shader_bind(shader, eye, window_width, window_height);
    float width = 20;

    // TODO voxel culling
    WorldDirectionalIterator itr = world_directional_iterator(
            world, eye, width, (width * window_height) / window_width, true, true, (int) (eye->z - VIEW_DEPTH)
    );
    while (world_directional_iterator_has_next(&itr)) {
        WorldTileData tile = world_directional_iterator_next(&itr);

        shader_set_tile_position(shader, tile.coord);

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

    shader_unbind(shader);
}
