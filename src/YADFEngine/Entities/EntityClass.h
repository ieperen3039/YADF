//
// Created by s152717 on 3-1-2020.
//

#ifndef YADF_ENTITYCLASS_H
#define YADF_ENTITYCLASS_H

#include "Callbacks.h"
#include "global.h"

typedef struct _Shader Shader;
typedef struct _Entity Entity;

typedef void (* UpdateFunction)(Entity* this);

struct EntityClassData {
    const char* name;
    UpdateFunction update;
    Visual* visual;
    int nr_of_materials;
    size_t metadata_size;
};

ENUM(EntityClass,
     NATURAL_WALL,
     GRASS,
)

void entity_class_init(VisualCreationFunction visual_create);

PURE const struct EntityClassData* entity_class_get(enum EntityClass type);

#endif //YADF_ENTITYCLASS_H
