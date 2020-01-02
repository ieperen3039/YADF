//
// Created by ieperen3039 on 29-11-19.
//

#ifndef YADF2_RENDER_H
#define YADF2_RENDER_H

#include "Camera.h"
#include "../DataStructures/Matrix4f.h"
#include "../DataStructures/Vector3f.h"
#include "../World/World.h"
#include "Mesh.h"
#include "Phongshader.h"
#include "../Entities/Entity.h"

static Mesh* cube_mesh = NULL;

static void render_frame(Phongshader* shader, World* world, Camera* camera, float aspect_ratio) {
    static float cam_angle = 1;
    cam_angle += 0.01f;
    camera_set_perspective(camera, cam_angle, 0.1f);
    Vector3f eye = camera_get_eye(camera);
    phong_bind(shader, &eye);

    // camera
    float w = camera_get_view_width(camera);
    Matrix4f viewProjection = matrix_get_ortho_projection(w, w / aspect_ratio, -w, w);
    Matrix4f cameraTransform = camera_get_transform(camera);
    matrix_mul(&viewProjection, &cameraTransform, &viewProjection);
    phong_set_view_projection_matrix(shader, &viewProjection);

    // light
    phong_add_light(shader, (Vector3f) {2, 1, 3}, (Color4f) {1, 1, 1, 1}, 1, true);
    phong_set_material(shader, 0, MATERIAL_PROPERTIES_DEFAULT);
    if (!cube_mesh) cube_mesh = mesh_from_file("res/models/cube.obj");

    // materials
    phong_set_material_a(shader, 0, COLOR_BLACK, COLOR_BLACK, 1);
    phong_set_material(shader, 1, MATERIAL_PROPERTIES_DEFAULT);

    // the view bounding box
    BoundingBox box = BOUNDING_BOX_EMPTY;
    bounding_box_get_view_box(&box, &viewProjection);
    box.zMax = (int) eye.z;

    WorldChunkIterator chunk_itr = world_get_chunk_iterator(world, box);
    while (world_chunk_iterator_has_next(&chunk_itr)) {
        WorldChunkData chunk = world_chunk_iterator_next(&chunk_itr);

        if (chunk.elt == NULL) {
            phong_set_tile_position(shader, chunk.coord);
            phong_set_tile_rotation(shader, 0);

            mesh_render(cube_mesh);
            continue;
        }

        WorldTileIterator tile_itr = chunk_get_tile_iterator(chunk.elt);
        while (chunk_tile_iterator_has_next(&tile_itr)) {
            WorldTileData tile = chunk_tile_iterator_next(&tile_itr);
            phong_set_tile_position(shader, tile.coord);

            if (tile.elt->flags & (TILE_FLAG_DISCOVERED)) {
                // render all entities
                ListIterator ety_itr = list_iterator(&tile.elt->entity_ptrs);
                while (list_iterator_has_next(&ety_itr)) {
                    Entity** ety_pp = list_iterator_next(&ety_itr);
                    Entity* ety = *ety_pp;
                    phong_set_tile_rotation(shader, ety->rotation);
                    entity_render(ety, shader);
                }

            } else {
                // render black cube
                mesh_render(cube_mesh);
            }

        }
    }

    phong_unbind(shader);
}

#endif //YADF2_RENDER_H
