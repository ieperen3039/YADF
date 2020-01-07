//
// Created by s152717 on 3-1-2020.
//

#ifndef YADF_ENTITYCLASS_H
#define YADF_ENTITYCLASS_H

#include <global.h>
#include <Entity.h>

void entity_class_init();

PURE const struct EntityClassData* entity_class_get(enum EntityClass type);

#endif //YADF_ENTITYCLASS_H
