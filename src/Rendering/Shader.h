//
// Created by ieperen3039 on 25-11-19.
//
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "global.h"

#ifndef YADF2_SHADER_H
#define YADF2_SHADER_H

/// @see #new_Shader(const char*, const char* const char*)
typedef GLuint ShaderID;

/**
 * Initialize a new shader object with the given shader texts. The returned pointer must be freed with #free_Shader(Shader*)
 * @return a pointer to an initialized shader object
 */
ShaderID shader_create(const char* vertex_shader_text, const char* fragment_shader_text, const char* geometry_shader_text);

/**
 * activates the given shader for rendering
 */
void shader_bind(ShaderID shader);

/**
 * deactivates any shader that is active
 */
void shader_unbind();

void shader_free(ShaderID shader);

#endif //YADF2_SHADER_H
