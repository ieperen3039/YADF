//
// Created by s152717 on 4-1-2020.
//

#include "EntityInstance.h"
#include "EntityClass.h"
#include "Materials.h"

Entity* entity_new(enum EntityClass type, const void* initial_data) {
    const struct EntityClassData* class_data = entity_class_get(type);

    Entity* new_entity = malloc(
            sizeof(Entity) +
            class_data->metadata_size
    );

    new_entity->type = type;
    new_entity->class = class_data;
    new_entity->flags = 0;

    void* data_ptr = new_entity + 1;
    if (class_data->metadata_size > 0) {
        assert(initial_data != NULL);
        new_entity->entity_data = data_ptr;
        memcpy(data_ptr, initial_data, class_data->metadata_size);

    } else {
        new_entity->entity_data = NULL;
    }

    return new_entity;
}

void world_tile_add_entity(WorldTileData tile, Entity* entity, WorldChunk* chunk) {
    entity->chunk = chunk;
    entity->position = tile.coord;
    list_add(&tile.elt->entity_ptrs, &entity);
}
