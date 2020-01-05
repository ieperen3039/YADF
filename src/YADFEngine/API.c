//
// Created by s152717 on 5-1-2020.
//

#include "API.h"
#include "World/WorldGenerator.h"
#include "World/UpdateEngine.h"

struct _YADFEngine {
    UpdateWorkerPool* workers;
    World* world;
    UpdateCycle game_time;
};

YADF_API YADFEngine* yadf_init(VisualCreationFunction create_visual) {
    LOG_INFO("Starting Simulator...");
    YADFEngine* engine = malloc(sizeof(YADFEngine));

    engine->workers = update_workers_new();

    LOG_INFO("Loading material and item properties");
    entity_class_init(create_visual);

    LOG_INFO("Generating world...");
    World* world = world_new(1000);
    if (!world) return NULL;

    // initialize starting area
    generator_generate_default(world, (BoundingBox) {-20, -20, -10, 20, 20, 10});
    engine->world = world;

    return engine;
}

YADF_API void yadf_trigger_loop_update(YADFEngine* engine) {
    update_world_tick(engine->world, engine->game_time, engine->workers);
}

YADF_API World* yadf_get_world(YADFEngine* engine) {
    return engine->world;
}

YADF_API void yadf_free(YADFEngine* engine) {
    update_workers_free(engine->workers);
    world_free(engine->world);
}