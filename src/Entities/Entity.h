//
// Created by s152717 on 27-12-2019.
//

#ifndef YADF_ENTITY_H
#define YADF_ENTITY_H

#include "../World/World.h"

//#define ENTITY_FLAG_LIVING (1 << 0)
//#define ENTITY_FLAG_ (1 << 1)
//#define ENTITY_FLAG_ (1 << 2)
//#define ENTITY_FLAG_ (1 << 3)
//#define ENTITY_FLAG_ (1 << 4)

typedef struct _Phongshader Phongshader;

struct EntityJumpTable {
    void (* update)(void* entity_data, WorldChunk* chunk, Vector3i* position);

    void (* render)(void* entity_data, Phongshader* shader);
};

typedef struct _Entity { // this struct is fail-fast (less bug-prone) if the first element is NOT a pointer
    int flags;
    void* entity_data; // the entity meta-data
    const struct EntityJumpTable* functions; // generic entity function pointers

    WorldChunk* chunk;
    Vector3i position;
    char rotation;
} Entity;

/**
 * Creates a new entity that uses the given jump table. This jump table is shared.
 * The metadata of this entity is set to a copy of the pointed data.
 * If initial_data is NULL, then the entity_data field will be NULL as well.
 * Do not try to reallocate the entity_data field.
 * @param data_size the size of the metadata
 * @param initial_data the initial metadata
 * @param jump_table the functionality of this entity (e.g. the methods)
 * @return the new entity
 */
static inline Entity* entity_new(const struct EntityJumpTable* jump_table, const void* initial_data, size_t data_size) {
    Entity* new_entity = malloc(sizeof(Entity) + data_size);

    new_entity->functions = jump_table;
    new_entity->flags = 0;

    if (data_size > 0 && initial_data != NULL) {
        void* data_ptr = new_entity + 1;
        new_entity->entity_data = data_ptr;
        memcpy(data_ptr, initial_data, data_size);

    } else {
        new_entity->entity_data = NULL;
    }

    return new_entity;
}

static inline void entity_place(Entity* ety, Vector3i position, WorldChunk* parent_chunk) {
    ety->position = position;
    ety->chunk = parent_chunk;
}

static inline void entity_update(Entity* ety) {
    ety->functions->update(ety->entity_data, ety->chunk, &ety->position);
}

static inline void entity_render(const Entity* ety, Phongshader* shader) {
    ety->functions->render(ety->entity_data, shader);
}

/**
 * Adds the given entity to the tile
 * @param tile the tile to add this entity's pointer to
 * @param the entity whomst pointer must be added to this list
 */
static inline void world_tile_add_entity(WorldTileData tile, Entity* entity, WorldChunk* chunk) {
    entity->chunk = chunk;
    entity->position = tile.coord;
    list_add(&tile.elt->entity_ptrs, &entity);
}

#ifndef DECLARE_ENTITY_STRUCT
// usage, assuming 'ety' as 'name' parameter:
// DECLARE_ENTITY_STRUCT(ety) { /* metadata struct of ety */ };
// DECLARE_ENTITY_FUNC(ety)
// this declares 'entity_ety_jt' and a number of declarations starting with 'ety'
// only ety_update(...) and ety_render(...) have yet to be implemented

    #define DECLARE_ENTITY_STRUCT(name) \
    struct Entity_##name##_metadata

    #define DECLARE_ENTITY_FUNC(name) \
    void name##_update(void* name##_data, WorldChunk* chunk, Vector3i* position);\
    void name##_render(void* name##_data, Phongshader* shader);\
    static const struct EntityJumpTable entity_##name##_jt = {name##_update, name##_render};\
    static Entity* name##_new(struct Entity_##name##_metadata* data){ return entity_new(&entity_##name##_jt, data, sizeof(struct Entity_##name##_metadata)); }
#endif

#endif //YADF_ENTITY_H
