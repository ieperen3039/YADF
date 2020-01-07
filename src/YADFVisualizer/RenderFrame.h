//
// Created by s152717 on 4-1-2020.
//

#ifndef YADF_RENDERFRAME_H
#define YADF_RENDERFRAME_H

#include <WorldAPI.h>
#include <Entity.h>
#include "Shader.h"
#include "Sprite.h"

void render_frame(Shader* shader, World* world, const Vector3f* eye, const Color4f* material_map,
                  const Sprite** entity_sprites, int window_width, int window_height);

#endif //YADF_RENDERFRAME_H
