//
// Created by ieperen3039 on 17-01-20.
//

#ifndef YADF_NATURALWALL_H
#define YADF_NATURALWALL_H

#include <Entity.h>
#include <EntityImpl.h>

int entity_impl_natural_wall_description(char* str, int max_chars, Entity* ety, enum DescriptionLength length) {
    struct EntityImplNaturalWall* data = ety->entity_data;
    switch (length) {
        case NAME:
            return snprintf(str, max_chars, "raw %s", material_name(data->material));
        case SHORT:
        case LONG:
            return snprintf(str, max_chars, "a natural wall made out of %s", material_name(data->material));
        default:
            assert(false);
    }
}

#endif //YADF_NATURALWALL_H
