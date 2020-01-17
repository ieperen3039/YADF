//
// Created by ieperen3039 on 17-01-20.
//

#ifndef YADF_ENTITYREAD_H
#define YADF_ENTITYREAD_H

#include <Entity.h>

/// appends the description of this specific entity
int entity_append_description(char* str, int max_chars, Entity* ety, enum DescriptionLength length) {
    ety->class->append_description(str, max_chars, ety, length);
}

bool is_vowel(char c){
    c = (char) tolower(c);
    return (c == 'e' || c == 'a' || c == 'o' || c == 'i' || c == 'u');
}

/// appends the name of this entity class, describing the given number if nr_of_elts is not 1
/// e.g. "an entity", "3 entities", "0 entities"
int entity_class_append_name(char* str, int max_chars, struct EntityClassData* entity_class, int nr_of_elts) {
    if (nr_of_elts == 1) {
        char* nomer = is_vowel(entity_class->name[0]) ? "an" : "a";
        return snprintf(
                str, max_chars, "%s %s",
                nomer, entity_class->name
        );
    } else {
        return snprintf(
                str, max_chars, "%d %s%s",
                nr_of_elts, entity_class->name, entity_class->plural_postfix
        );
    }
}

#endif //YADF_ENTITYREAD_H
