//
// Created by s152717 on 3-1-2020.
//

#include "EntityClass.h"
#include "EntityImpl.h"
#include "EntityDerivations/NaturalWall.h"
#include "EntityDerivations/VegetationPatch.h"
#include "EntityDerivations/Template.h"
#include "EntityDerivations/Boid.h"

struct EntityClassData type_data[ENUM_SIZE(EntityClass)] = {}; // initialize NULL
bool is_init = false;

/** UpdateFunction that does nothing */
void entity_impl_empty_update(Entity* this, UpdateCycle time) {}

/** ApplicationFunction that does nothing */
void entity_impl_empty_apply(Entity* this, Vector3ic* coordinate, WorldTile* tile) {}

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
                data->description = "raw block of material that covers the entire tile";
                data->append_description = entity_impl_natural_wall_description;
                data->metadata_size = sizeof(struct EntityImplNaturalWall);
                data->pre_update = entity_impl_empty_update;
                data->post_update = entity_impl_empty_apply;
                data->flags = ENTITY_FLAG_BLOCKING;
                break;

            case VEGETATION_PATCH:
                data->name = "patch";
                data->plural_postfix = "es";
                data->description = "sprout of vegetation that covers the floor of the tile";
                data->metadata_size = sizeof(struct EntityImplGrass);
                data->append_description = entity_impl_vegetation_patch_description;
                data->pre_update = entity_impl_empty_update;
                data->post_update = entity_impl_empty_apply;
                data->flags = 0;
                break;

            case BOID:
                data->name = "boid";
                data->plural_postfix = "s";
                data->description = "<debug object>";
                data->metadata_size = sizeof(struct EntityImplBoid);
                data->append_description = entity_impl_boid_description;
                data->pre_update = entity_impl_boid_update;
                data->post_update = entity_impl_boid_apply;
                data->flags = 0;
                break;
        }

        assert(data->name != NULL);
        assert(data->plural_postfix != NULL);
        assert(data->description != NULL);
        assert(data->append_description != NULL);
        assert(data->pre_update != NULL);
        assert(data->post_update != NULL);
    }

    is_init = true;
}

PURE const struct EntityClassData* entity_class_get(enum EntityClass type) {
    assert(is_init);
    return &type_data[type];
}