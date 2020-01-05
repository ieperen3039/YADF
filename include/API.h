//
// Created by s152717 on 5-1-2020.
//

#ifndef YADF_API_H
#define YADF_API_H

#include "APIDefine.h"
#include "Callbacks.h"

#include "../src/YADFEngine/World/World.h"

typedef struct _YADFEngine YADFEngine;

/**
 * initializes the world
 */
YADF_API YADFEngine* yadf_init(VisualCreationFunction create_visual);

YADF_API void yadf_trigger_loop_update(YADFEngine* engine);

YADF_API World* yadf_get_world(YADFEngine* engine);

YADF_API void yadf_free(YADFEngine* engine);

#endif //YADF_API_H
