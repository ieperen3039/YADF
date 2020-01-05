//
// Created by ieperen3039 on 04-12-19.
//

#ifndef YADF_MATERIALPROPERTIES_H
#define YADF_MATERIALPROPERTIES_H

#include "global.h"
#include "../YADFEngine/DataStructures/Structs.h"
#include "../YADFEngine/Entities/Materials.h"

#include "../YADFEngine/DataStructures/List.h"

ErrorCode material_read_json(const char* file, Color4f* material_colors);

#endif //YADF_MATERIALPROPERTIES_H
