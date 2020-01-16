//
// Created by s152717 on 5-1-2020.
//

#include "YADFEngine.h"
#include "World/WorldGenerator.h"
#include "UpdateEngine/UpdateEngine.h"
#include "Entities/EntityClass.h"

struct _YADFEngine {
    UpdateWorkerPool* workers;
    World* world;
};

YADF_API YADFEngine* yadf_init() {
    YADFEngine* engine = malloc(sizeof(YADFEngine));

    LOG_INFO("Loading material and item properties");
    entity_class_init();

    LOG_INFO("Generating world...");
    World* world = world_new(1000);
    if (!world) return NULL;

    // initialize starting area
    generator_generate_default(world, (BoundingBox) {-20, -20, -10, 20, 20, 10});
    engine->world = world;

    LOG_INFO("Booting worker threads...");
    engine->workers = update_workers_new();

    return engine;
}

const struct EntityClassData* yadf_get_entity_class(enum EntityClass entity_class) {
    return entity_class_get(entity_class);
}

YADF_API void yadf_trigger_loop_update(YADFEngine* engine) {
    update_start_tick(engine->workers, engine->world);
}

YADF_API World* yadf_get_world(YADFEngine* engine) {
    return engine->world;
}

YADF_API void yadf_free(YADFEngine* engine) {
    update_workers_free(engine->workers);
    world_free(engine->world);
}