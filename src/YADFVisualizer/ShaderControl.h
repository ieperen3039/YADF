//
// Created by s152717 on 4-1-2020.
//

#ifndef YADF_SHADERCONTROL_H
#define YADF_SHADERCONTROL_H

#include "global.h"
#include "../src/YADFEngine/DataStructures/Matrix4f.h"
#include "../src/YADFVisualizer/External/GL/glew.h"
#include "../src/YADFEngine/DataStructures/List.h"
#include "../src/YADFEngine/Entities/Materials.h"
#include "../src/YADFVisualizer/MaterialProperties.h"

typedef struct _Shader Shader;

/**
 * Create the shader
 * @param material_properties an array of MaterialSize, containing the color of each material
 * @return a new shader object
 */
Shader* shader_new(Color4f* material_properties);

/**
 * initializes this shader to draw a single frame.
 * @param shader the shader object
 * @param camera_eye the position of the camera
 */
void shader_bind(Shader* shader, Vector3fc* camera_eye);

/**
 * ends a single frame, detaching this shader
 * @param shader the shader currently bound using shader_bind
 */
void shader_unbind(Shader* shader);

void shader_free(Shader* shader);

/** @return the GL program id of this shader */
PURE GLuint shader_id(Shader* shader);

#endif //YADF_SHADERCONTROL_H
