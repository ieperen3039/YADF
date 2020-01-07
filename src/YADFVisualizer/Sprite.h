//
// Created by s152717 on 5-1-2020.
//

#ifndef YADF_SPRITE_H
#define YADF_SPRITE_H

#include "Shader.h"

typedef struct _Sprite Sprite;

void sprite_init();

Sprite* sprite_new(const char* filename);

void sprite_draw(const Sprite* sprite, Shader* shader);

#endif //YADF_SPRITE_H
