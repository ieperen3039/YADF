//
// Created by s152717 on 31-12-2019.
//

#include "UpdateEngine.h"

UpdateWorkerPool* update_workers_new() {
    return NULL;
}

void update_workers_free(UpdateWorkerPool* pool) {
}

void update_world_tick(World* world, UpdateCycle game_time, UpdateWorkerPool* worker_pool) {
    List* src_list = world_get_entities_to_update(world);
    ListIterator entities = list_iterator(src_list);

    while (list_iterator_has_next(&entities)) {
        Entity** ety = list_iterator_next(&entities);
        entity_update(*ety);
    }

    list_clear(src_list);
}
