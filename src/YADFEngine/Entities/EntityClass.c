//
// Created by s152717 on 3-1-2020.
//

#include "EntityClass.h"
#include "EntityImplStatic.h"
#include "../DataStructures/List.h"
#include "Callbacks.h"

struct EntityClassData type_data[ENUM_SIZE(EntityClass)];
bool is_init = false;

void entity_class_init(VisualCreationFunction visual_create) {
    if (is_init) return;

    for (int i = 0; i < EntityClassSize; ++i) {
        enum EntityClass type = EntityClassValues[i];

        struct EntityClassData* data = &type_data[type];

        switch (type) {
            case NATURAL_WALL:
                data->name = "natural wall";
                data->metadata_size = sizeof(struct EntityImplNaturalWall);
                data->update = entity_impl_natural_wall_update;
                data->visual = visual_create(data->name);
                data->nr_of_materials = 1;
                break;

            case GRASS:
                data->name = "grass";
                data->metadata_size = sizeof(struct EntityImplGrass);
                data->update = entity_impl_grass_update;
                data->visual = visual_create(data->name);
                data->nr_of_materials = 2;
                break;

            default: assert(false);
        }
    }

    is_init = true;
}

PURE const struct EntityClassData* entity_class_get(enum EntityClass type) {
    assert(is_init);
    return &type_data[type];
}