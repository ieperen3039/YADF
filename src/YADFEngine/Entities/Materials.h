//
// Created by s152717 on 4-1-2020.
//

#ifndef YADF_MATERIALS_H
#define YADF_MATERIALS_H

#include "global.h"

typedef enum Material Material;
ENUM( Material ,
    SLATE,
    BASALT
)

PURE static const char* material_name(Material material){
    // basically a lookup table
    switch (material){
        case SLATE:
            return "slate";
        case BASALT:
            return "basalt";
        default: return "non-existent material";
    }
}

#endif //YADF_MATERIALS_H
