//
// Created by s152717 on 3-1-2020.
//

#include "EntityClass.h"
#include "EntityImpl.h"
#include "EntityUpdateStatic.h"

struct EntityClassData type_data[ENUM_SIZE(EntityClass)];
bool is_init = false;

void entity_class_init() {
    if (is_init) return;

    for (int i = 0; i < EntityClassSize; ++i) {
        enum EntityClass type = EntityClassValues[i];

        struct EntityClassData* data = &type_data[type];
        data->type = type;

        switch (type) {
            default:
                assert(false);
            case NATURAL_WALL:
                data->name = "natural wall";
                data->plural_postfix = "s";
                data->description = "block of material that covers the entire tile";
                data->metadata_size = sizeof(struct EntityImplNaturalWall);
                data->pre_update = entity_impl_empty_update;
                data->post_update = entity_impl_no_apply;
                break;

            case VEGETATION_PATCH:
                data->name = "patch";
                data->plural_postfix = "es";
                data->description = "sprout of vegetation that covers the floor of the tile";
                data->metadata_size = sizeof(struct EntityImplGrass);
                data->pre_update = entity_impl_empty_update;
                data->post_update = entity_impl_no_apply;
                break;

            case CREATURE:
                data->name = "creature";
                data->plural_postfix = "s";
                data->description = "living being";
                data->metadata_size = 0;
                data->pre_update = entity_impl_empty_update;
                data->post_update = entity_impl_no_apply;
                break;
        }
    }

    is_init = true;
}

PURE const struct EntityClassData* entity_class_get(enum EntityClass type) {
    assert(is_init);
    return &type_data[type];
}