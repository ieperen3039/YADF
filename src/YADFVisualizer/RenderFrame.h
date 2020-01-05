//
// Created by s152717 on 4-1-2020.
//

#ifndef YADF_RENDERFRAME_H
#define YADF_RENDERFRAME_H

#include "ShaderControl.h"
#include "Visual.h"
#include "../YADFEngine/Entities/Entity.h"

void render_frame(Shader* shader, World* world, Camera* camera, float aspect_ratio) {
    // camera
    const float cam_angle = 0.2f;
    camera_set_perspective(camera, cam_angle, 0.05f);
    Vector3f eye = camera_get_eye(camera);

    shader_bind(shader, &eye);

    float h = camera_get_view_size(camera);
    float w = h * aspect_ratio;
    Matrix4f viewProjection = matrix_get_ortho_projection(w, h, -h, w + 100);
    Matrix4f cameraTransform = camera_get_transform(camera);
    matrix_mul(&viewProjection, &cameraTransform, &viewProjection);

    shader_set_view_projection_matrix(shader, &viewProjection);

    BoundingBox box = BOUNDING_BOX_EMPTY;
    {
        // the view bounding box
        bounding_box_get_view_box(&box, &viewProjection);
        box.zMax = (int) eye.z;
    }

    // light
    shader_add_light(shader, (Vector3f) {2, 1, 3}, (Color4f) {1, 1, 1, 1}, 1, true);

    static Visual* unknown_chunk_visual;
    if (!unknown_chunk_visual) unknown_chunk_visual = visual_create("unknown");

    WorldChunkIterator chunk_itr = world_get_chunk_iterator(world, box);
    while (world_chunk_iterator_has_next(&chunk_itr)) {
        WorldChunkData chunk = world_chunk_iterator_next(&chunk_itr);

        if (chunk.elt == NULL) {
            shader_set_tile_position(shader, chunk.coord);
            shader_set_tile_rotation(shader, 0);

//            visual_draw(unknown_chunk_visual, shader);
            continue;
        }

        WorldTileIterator tile_itr = chunk_get_tile_iterator(chunk.elt);
        while (chunk_tile_iterator_has_next(&tile_itr)) {
            WorldTileData tile = chunk_tile_iterator_next(&tile_itr);

            shader_set_tile_position(shader, tile.coord);

            if (tile.elt->flags & (TILE_FLAG_DISCOVERED)) {
                // render all entities
                ListIterator ety_itr = list_iterator(&tile.elt->entity_ptrs);
                while (list_iterator_has_next(&ety_itr)) {
                    Entity** ety_pp = list_iterator_next(&ety_itr);
                    Entity* entity = *ety_pp;

                    for (int i = 0; i < entity->class->nr_of_materials; ++i) {
                        shader_set_material(shader, i, entity->materials[i]);
                    }

                    Visual* element = entity->class->visual;
                    visual_draw(element, shader);
                }

            } else {
                // render black cube
                visual_draw(unknown_chunk_visual, shader);
            }
        }
    }

    shader_unbind(shader);
}

#endif //YADF_RENDERFRAME_H
