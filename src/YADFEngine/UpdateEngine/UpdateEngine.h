//
// Created by s152717 on 13-12-2019.
//

#ifndef YADF_UPDATEENGINE_H
#define YADF_UPDATEENGINE_H

#define TEMPERATURE_LEVEL 1
#define PRESSURE_LEVEL 3

#include "../World/World.h"

typedef struct _UpdateWorkerPool UpdateWorkerPool;

/**
 * create a new worker pool for updating the world state
 * @return an initialized worker pool
 */
UpdateWorkerPool* update_workers_new(void);

/**
 * Destroys a worker pool, waiting for them to finish their current work.
 * @param pool a pool created using update_workers_new
 */
void update_workers_free(UpdateWorkerPool* pool);

/**
 * Starts a single update. This method must be called by the render thread, to allow synchronization
 * @param world the world to update
 * @param game_time the current game time
 * @param pool the available workers
 */
void update_start_tick(UpdateWorkerPool* pool, World* world);


#endif //YADF_UPDATEENGINE_H
