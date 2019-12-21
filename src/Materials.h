//
// Created by ieperen3039 on 04-12-19.
//

#ifndef YADF_MATERIALS_H
#define YADF_MATERIALS_H

enum Material {
    BASALT,
    GRANITE,
    DIORITE,
    ANDESITE,
    SLATE,
    MARBLE,
    QUARTZ
};

typedef struct {
    Color4f diffuse;
    Color4f specular;
    float reflectance;
} MaterialProperties;

static const MaterialProperties material_properties_default = {
        {0.8, 0.8, 0.8, 1.0},
        {1, 1, 1, 1},
        20,
};

#endif //YADF_MATERIALS_H
