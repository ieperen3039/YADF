//
// Created by s152717 on 5-1-2020.
//

#ifndef YADF_YADFENGINE_H
#define YADF_YADFENGINE_H

#include "APIDefine.h"
#include "Entity.h"
#include "WorldAPI.h"

typedef struct _YADFEngine YADFEngine;

/**
 * initializes the world
 */
YADF_API YADFEngine* yadf_init(void);

/**
 * @param engine the game instance
 * @return the world data by reference
 */
YADF_API World* yadf_get_world(YADFEngine* engine);

YADF_API void yadf_trigger_loop_update(YADFEngine* engine);

YADF_API void yadf_free(YADFEngine* engine);

YADF_API const struct EntityClassData* yadf_get_entity_class(enum EntityClass entity_class);

#endif //YADF_YADFENGINE_H
