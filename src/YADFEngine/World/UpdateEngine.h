//
// Created by s152717 on 13-12-2019.
//

#ifndef YADF_UPDATEENGINE_H
#define YADF_UPDATEENGINE_H

#define TEMPERATURE_LEVEL 1
#define PRESSURE_LEVEL 3

#include "World.h"

typedef struct _UpdateWorkerPool UpdateWorkerPool;

/**
 * create a new worker pool for updating the world state
 * @return an initialized worker pool
 */
UpdateWorkerPool* update_workers_new();

/**
 * Destroys a worker pool, waiting for them to finish their current work.
 * @param pool a pool created using update_workers_new
 */
void update_workers_free(UpdateWorkerPool* pool);

/**
 * Updates a single world tick.
 * This method takes the minimum time required to distribute the work, and returns possibly before the update has finished.
 * This might mean that all work is done in this method
 * @param world the world to update
 * @param game_time the current game time
 * @param worker_pool the available workers
 */
void update_world_tick(World* world, UpdateCycle game_time, UpdateWorkerPool* worker_pool);

#endif //YADF_UPDATEENGINE_H
