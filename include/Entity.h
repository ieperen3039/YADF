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
     BOID, // it is like a bird, but then more irish
)

enum DescriptionLength {
    // "You punch the <NAME>" / "The <NAME> punches back!"
    NAME,
    // "This is <SHORT>" / "You found a <SHORT>!" (excludes a/an/the)
    SHORT,
    // "This <NAME> is <LONG>"
    LONG
};

typedef void (* UpdateFunction)(Entity* this, UpdateCycle game_time);

typedef void (* ApplicationFunction)(Entity* this, Vector3ic* coordinate, WorldTile* tile);
// appends the description of the entity to the given string.
typedef int (* DescriptionAppendFunction)(char* str, int max_chars, Entity* ety, enum DescriptionLength length);


struct EntityClassData {
    // singular generic name of an object of this class (e.g. "entity")
    const char* name;
    // postfix for plural, possibly starting with backspaces (\b) (e.g. "\bies")
    const char* plural_postfix;
    // generic description, leaving out "a <name> is a "
    // (e.g. "thing that physically exists. This includes things like ghosts and dust, but not entity statuses, gasses, fluids or fire")
    const char* description;
    // update step that can read but not write public data
    UpdateFunction pre_update;
    // update step that can write but not read public data
    ApplicationFunction post_update;
    // function that appends a description of this specific entity to the given char
    DescriptionAppendFunction append_description;
    // sizeof(Entity::entity_data)
    size_t metadata_size;
    // the type that uniquely defines this class
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
