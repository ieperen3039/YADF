//
// Created by s152717 on 5-1-2020.
//

#ifndef YADF_ENTITY_H
#define YADF_ENTITY_H

#include "global.h"
#include "Structs.h"
#include "WorldAPI.h"

typedef struct _Entity Entity;
typedef enum Material Material;
typedef struct _WorldChunk WorldChunk;

typedef void (* UpdateFunction)(Entity* this, UpdateCycle game_time);
typedef void (* ApplicationFunction)(Entity* this, Vector3ic* coordinate, WorldTile* tile);

struct EntityClassData {
    const char* name;
    UpdateFunction pre_update;
    ApplicationFunction post_update;
    size_t metadata_size;
};

ENUM(EntityClass,
     NATURAL_WALL,
     GRASS,
     CREATURE,
)

typedef struct _Entity { // this struct is fail-fast (less bug-prone) if the first element is NOT a pointer
    enum EntityClass type;
    int flags;
    void* entity_data; // the entity meta-data

    const struct EntityClassData* class;

    WorldChunk* chunk;
    Vector3i position;
    Vector3i updatePosition; // may be equal to position
} Entity;

#endif //YADF_ENTITY_H
