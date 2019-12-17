//
// Created by ieperen3039 on 25-11-19.
//

#ifndef YADF2_SHADER_H
#define YADF2_SHADER_H

#include <malloc.h>
//
// Created by ieperen3039 on 25-11-19.
//
#include "global.h"

/// @see #new_Shader(const char*, const char* const char*)
typedef GLuint ShaderID;

static inline void printShaderLog(GLuint shader) {
    GLint logSize = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
    char* msg = (char*) malloc(logSize);
    glGetShaderInfoLog(shader, logSize, NULL, msg);
    LOG_INFO_F("Shader log: %s", msg);
    free(msg);
}

static inline void printProgramLog(GLuint program) {
    GLint logSize = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
    char* msg = (char*) malloc(logSize);
    glGetProgramInfoLog(program, logSize, NULL, msg);
    LOG_INFO_F("Program validation log: %s", msg);
    free(msg);
}

static inline GLuint createShader(const char* vertex_shader_text, int type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &vertex_shader_text, NULL);
    glCompileShader(shader);

    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        LOG_ERROR("Error in shader");
        printShaderLog(shader);
        return 0;
    }

    return shader;
}

static ShaderID shader_create(const char* vertex_shader_text, const char* fragment_shader_text, const char* geometry_shader_text) {
    // create available shaders
    GLuint vertex_shader = createShader(vertex_shader_text, GL_VERTEX_SHADER);
    if (!vertex_shader) {
        LOG_INFO("Failed to create vertex shader: aborting.");
        return 0;
    }

    GLuint fragment_shader = createShader(fragment_shader_text, GL_FRAGMENT_SHADER);
    if (!fragment_shader) {
        LOG_INFO("Failed to create fragment shader: aborting.");
        return 0;
    }

    GLuint geometry_shader = 0;
    if (geometry_shader_text) {
        geometry_shader = createShader(geometry_shader_text, GL_GEOMETRY_SHADER);
        if (!geometry_shader) {
            LOG_INFO("Failed to create geometry shader: aborting.");
            return 0;
        }
    }

    // attach shaders to program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    if (geometry_shader) glAttachShader(program, geometry_shader);

    glLinkProgram(program);

    // cleanup shaders after linking
    glDetachShader(program, vertex_shader);
    glDeleteShader(vertex_shader);
    glDetachShader(program, fragment_shader);
    glDeleteShader(fragment_shader);
    if (geometry_shader) {
        glDetachShader(program, geometry_shader);
        glDeleteShader(geometry_shader);
    }

    glValidateProgram(program);
    printProgramLog(program);

    GLint status = 0;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
    if (status != GL_TRUE) {
        LOG_ERROR("Error in program validation");
    }

    return program;
}

#endif //YADF2_SHADER_H