//
// Created by ieperen3039 on 17-01-20.
// find/replace all occurrences of 'template' with the new class name
//

#ifndef YADF_TEMPLATE_H
#define YADF_TEMPLATE_H

#include <Entity.h>
#include <EntityImpl.h>

int entity_impl_template_description(char* str, int max_chars, Entity* ety, enum DescriptionLength length) {
    struct EntityImpl* data = ety->entity_data;
    switch (length) {
        case NAME:
        case SHORT:
        case LONG:
            return snprintf(str, max_chars, "<template>");
        default:
            assert(false);
    }
}

#endif //YADF_TEMPLATE_H
