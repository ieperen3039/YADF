//
// Created by s152717 on 4-1-2020.
//

#ifndef YADF_WINDOW_H
#define YADF_WINDOW_H

#include "global.h"
#include "../YADFEngine/World/World.h"
#include "External/GL/glew.h"

#include <GLFW/glfw3.h>

typedef struct _Visualizer Visualizer;

Visualizer* visualizer_create_window();

bool visualizer_has_error(Visualizer* vis);

bool visualizer_is_closed(Visualizer* vis);

void visualizer_free(Visualizer* vis);

void visualizer_callbacks(Visualizer* vis);

void visualizer_draw_frame(Visualizer* vis, World* world);

#endif //YADF_WINDOW_H
