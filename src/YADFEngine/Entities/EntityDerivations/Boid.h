//
// Created by ieperen3039 on 17-01-20.
//

#ifndef YADF_BOID_H
#define YADF_BOID_H

#include <Entity.h>
#include <EntityImpl.h>

int entity_impl_boid_description(char* str, int max_chars, Entity* ety, enum DescriptionLength length) {
    struct EntityImplBoid* data = ety->entity_data;
    switch (length) {
        case DESC_NAME:
        case DESC_SHORT:
        case DESC_LONG:
            return snprintf(str, max_chars, "boid %d", data->id);
        default:
            assert(false);
    }
}

void entity_impl_boid_update(Entity* this, UpdateCycle game_time){

}

void entity_impl_boid_apply(Entity* this, Vector3ic* coordinate, WorldTile* tile){

}

#endif //YADF_BOID_H
