//
// Created by s152717 on 3-1-2020.
//

#include "EntityClass.h"
#include "EntityImpl.h"
#include "EntityUpdateStatic.h"
#include "../DataStructures/List.h"

struct EntityClassData type_data[ENUM_SIZE(EntityClass)];
bool is_init = false;

void entity_class_init() {
    if (is_init) return;

    for (int i = 0; i < EntityClassSize; ++i) {
        enum EntityClass type = EntityClassValues[i];

        struct EntityClassData* data = &type_data[type];

        switch (type) {
            case NATURAL_WALL:
                data->name = "natural wall";
                data->metadata_size = sizeof(struct EntityImplNaturalWall);
                data->pre_update = entity_impl_natural_wall_update;
                data->post_update = entity_impl_natural_wall_apply;
                break;

            case GRASS:
                data->name = "grass";
                data->metadata_size = sizeof(struct EntityImplGrass);
                data->pre_update = entity_impl_grass_update;
                data->post_update = entity_impl_grass_apply;
                break;

            default: assert(false);
            case CREATURE:
                data->name = "creature"; // TODO valid update
                data->metadata_size = sizeof(struct EntityImplGrass);
                data->pre_update = entity_impl_grass_update;
                break;
        }
    }

    is_init = true;
}

PURE const struct EntityClassData* entity_class_get(enum EntityClass type) {
    assert(is_init);
    return &type_data[type];
}