//
// Created by ieperen3039 on 07-01-20.
//

// (once): await loop trigger
// (once): lock callbacks
// (per entity): calculate new state without writing, and determine coordinate to modify (update_entities_pre)
// (once): group entities on position (update_sort_entities_on_position)
// (once): lock frame rendering
// (per tile): modify the chosen tile || modify own tile and own state (update_entities_post)
// (once): unlock callbacks, unlock frame rendering

#ifdef YADF_UPDATEENGINESHARED_H
#error There should be only one update engine in a project
#endif
#define YADF_UPDATEENGINESHARED_H

#include <Entity.h>
#include "../Entities/EntityInstance.h"

void update_entities_pre(ListIterator entities, UpdateCycle game_time) {
    while (list_iterator_has_next(&entities)) {
        Entity** ety = list_iterator_next(&entities);
        Entity* entity = *ety;

        entity->class->pre_update(entity, game_time);
    }
}

int update_hash(Vector3i pos) {
    int hash = (pos.x << CHUNK_COORD_BITS) ^(pos.y << CHUNK_COORD_BITS) ^(pos.z << CHUNK_COORD_BITS);
    return hash;
}

struct EntityUpdateElt {
    Entity* ety;
    Vector3i* pos;
};

/**
 * initializes the lists to contain elements of struct EntityUpdateElt.
 * @param lists an array of lists to initialize
 * @param nr_of_lists the number of lists in lists, at least equal to the number of workers
 * @param nr_of_entities the number of entities that will be stored in these lists in total
 */
void update_init_sort_lists(List* lists, int nr_of_lists, int nr_of_entities) {
    int capacity = (int) (1.5 * nr_of_entities / nr_of_lists) + 1;
    for (int i = 0; i < nr_of_lists; ++i) {
        list_init(&lists[i], sizeof(struct EntityUpdateElt), capacity);
    }
}

/**
 * adds all entities to the given lists, such that each tile coordinate is only contained in one list
 * @param entities an iterator of Entity*
 * @param lists an array of lists to place the entities in, initialized by update_init_sort_lists
 * @param nr_of_lists the number of lists in the given array
 */
void update_sort_entities_on_position(ListIterator entities, List* lists, int nr_of_lists) {
    while (list_iterator_has_next(&entities)) {
        Entity** ety_pp = list_iterator_next(&entities);
        Entity* entity = *ety_pp;
        struct EntityUpdateElt elt = {entity, NULL};

        int p_hash = update_hash(entity->position);
        elt.pos = &entity->position;
        list_add(&lists[p_hash % nr_of_lists], &elt);

        int t_hash = update_hash(entity->updatePosition);
        elt.pos = &entity->updatePosition;
        list_add(&lists[t_hash % nr_of_lists], &elt);
    }
}

/**
 * call this on each list of update_sort_entities_on_position
 * @param entities an iterator of struct EntityUpdateElt
 */
void update_entities_post(ListIterator entities) {
    while (list_iterator_has_next(&entities)) {
        struct EntityUpdateElt* element = list_iterator_next(&entities);
        Entity* entity = element->ety;

        WorldTile* tile = world_get_tile_from_chunk(entity->chunk, *element->pos);
        entity->class->post_update(entity, element->pos, tile);
    }
}
