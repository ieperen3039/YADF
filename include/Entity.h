//
// Created by s152717 on 5-1-2020.
// Minimum required information to use Entities and EntityClasses
//

#ifndef YADF_ENTITY_H
#define YADF_ENTITY_H

#include "global.h"
#include "Structs.h"
#include "WorldAPI.h"

typedef struct _Entity Entity;
typedef struct _WorldChunk WorldChunk;

ENUM( EntityClass,
     NATURAL_WALL,
     VEGETATION_PATCH,
     CREATURE,
)

typedef void (* UpdateFunction)(Entity* this, UpdateCycle game_time);
typedef void (* ApplicationFunction)(Entity* this, Vector3ic* coordinate, WorldTile* tile);

struct EntityClassData {
    const char* name; // singular generic name of an object of this class (e.g. "entity")
    const char* plural_postfix; // postfix for plural, possibly starting with backspaces (\b) (e.g. "\bies")
    const char* description; // generic description, leaving out "a <name> is a "
        // (e.g. "thing that physically exists. This includes things like ghosts, gasses, fluids and fire, but not light or images")
    UpdateFunction pre_update; // update step that can read but not write public data
    ApplicationFunction post_update; // update step that can write but not read public data
    size_t metadata_size; // sizeof(Entity::entity_data)
    enum EntityClass type;
};

typedef struct _Entity { // this struct is fail-fast (less bug-prone) if the first element is NOT a pointer
    int flags;
    void* entity_data; // the entity meta-data

    const struct EntityClassData* class;

    WorldChunk* chunk;
    Vector3i position;
    Vector3i updatePosition; // may be equal to position
} Entity;

#endif //YADF_ENTITY_H
