//
// Created by s152717 on 5-1-2020.
//

#ifndef YADF_CALLBACKS_H
#define YADF_CALLBACKS_H

typedef struct _Visual Visual;
/// a function that returns a 'visual' of this entity, which is then stored per entity of this class
typedef Visual* (VisualCreationFunction)(const char* entity_name);

#endif //YADF_CALLBACKS_H
