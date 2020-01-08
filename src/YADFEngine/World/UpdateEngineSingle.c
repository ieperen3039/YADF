//
// Created by s152717 on 31-12-2019.
//

#include "UpdateEngine.h"
#include "UpdateEngineShared.h"

UpdateWorkerPool* update_workers_new() {
    return NULL;
}

void update_workers_free(UpdateWorkerPool* pool) {
}

void update_world_tick(World* world, UpdateCycle game_time, UpdateWorkerPool* null) {
    List* src_list = world_get_entities_to_update(world);

    update_entities_pre(list_iterator(src_list), game_time);
    
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
    
    update_entities_post(list_iterator(&all));

    list_clear(src_list);
}
