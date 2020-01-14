//
// Created by s152717 on 31-12-2019.
//

#include "UpdateEngine.h"
#include "UpdateEngineShared.h"

struct _UpdateWorkerPool{
    List all;
    UpdateCycle game_time;
};

UpdateWorkerPool* update_workers_new() {
    UpdateWorkerPool* pool = malloc(sizeof(UpdateWorkerPool));
    pool->game_time = 0;
    list_init(&pool->all, sizeof(struct EntityUpdateElt), 128);
    return NULL;
}

void update_workers_free(UpdateWorkerPool* pool) {
    list_free(&pool->all);
    free(pool);
}

void update_start_tick(UpdateWorkerPool* pool, World* world) {
    List* src_list = world_get_entities_to_update(world);

    ListIterator entities = list_iterator(src_list);
    update_entities_pre(&entities, pool->game_time++);
    
    List all;
    list_init(&all, sizeof(struct EntityUpdateElt), list_get_size(src_list));

    int size = list_get_size(&all);
    for (int i = 0; i < size; ++i) {
        Entity** ety_pp = list_get(src_list, i);
        Entity* entity = *ety_pp;
        struct EntityUpdateElt elt = {entity, NULL};

        elt.pos = &entity->position;
        list_add(&all, &elt);

        elt.pos = &entity->updatePosition;
        list_add(&all, &elt);
    }

    ListIterator iterator = list_iterator(&all);
    update_entities_post(&iterator);

    list_clear(src_list);
}
