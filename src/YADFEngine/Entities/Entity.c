//
// Created by s152717 on 4-1-2020.
//

#include "Entity.h"

Entity* entity_new(enum EntityClass type, const void* initial_data, Material* materials) {
    const struct EntityClassData* class_data = entity_class_get(type);

    size_t material_array_data_size = class_data->nr_of_materials * sizeof(Material);

    Entity* new_entity = malloc(
            sizeof(Entity) +
            material_array_data_size +
            class_data->metadata_size
    );

    new_entity->type = type;
    new_entity->class = class_data;
    new_entity->flags = 0;

    void* material_ptr = new_entity + 1;
    if (material_array_data_size > 0){
        assert(materials != NULL);
        memcpy(material_ptr, materials, material_array_data_size);
        new_entity->materials = material_ptr;

    } else {
        new_entity->materials = NULL;
    }

    void* data_ptr = material_ptr + material_array_data_size;
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
