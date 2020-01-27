//
// Created by ieperen3039 on 12-12-19.
//

#include "Shader.h"
#include "../YADFEngine/Tools.h"

#include <GL/glew.h>
#include <malloc.h>

#define SHADER_MAX_NUM_MATERIALS 8 // materials per entity
#define SPRITE_SPINE_PIXELS 24.0
#define SPRITE_BOWEL_PIXELS 19.0

struct _Shader {
    GLuint ID;
    GLuint texture_position;
    GLuint camera_position;
    GLuint scaling;
    GLuint brightness;
    GLuint sprite;
    Vector3f camera_vector3f;
    GLuint colors[SHADER_MAX_NUM_MATERIALS];
};

static GLuint
create_program(const char* vertex_shader_text, const char* fragment_shader_text, const char* geometry_shader_text);

Shader* shader_new() {
    char* vertex_shader = tool_read_file("res/tile_shader.vert", NULL);
    char* fragment_shader = tool_read_file("res/tile_shader.frag", NULL);
    if (vertex_shader == NULL || fragment_shader == NULL) return NULL;

    GLuint shader = create_program(vertex_shader, fragment_shader, NULL);
    if (!shader) return NULL;

    Shader* p = malloc(sizeof(Shader));
    p->ID = shader;
    p->sprite = glGetUniformLocation(shader, "sprite");
    p->texture_position = glGetUniformLocation(shader, "sprite_origin");
    p->camera_position = glGetUniformLocation(shader, "camera_position");
    p->scaling = glGetUniformLocation(shader, "scaling");
    p->brightness = glGetUniformLocation(shader, "brightness");

    for (int i = 0; i < SHADER_MAX_NUM_MATERIALS; ++i) {
        char uniform_name[15];
        snprintf(uniform_name, 15, "colors[%d]", i);
        p->colors[i] = glGetUniformLocation(shader, uniform_name);
    }

    assert(p->sprite != -1);
    assert(p->texture_position != -1);
    assert(p->camera_position != -1);
    assert(p->scaling != -1);

    return p;
}

void shader_bind(Shader* shader, Vector3fc* eye, int window_width, int window_height) {
    shader->camera_vector3f = *eye;
    glUseProgram(shader->ID);

    const double zoom_scaling = 1;
    double x_scale = zoom_scaling * SPRITE_WIDTH_PIXELS / window_width;
    double y_scale = zoom_scaling * SPRITE_HEIGHT_PIXELS / window_height;
    glUniform2f(shader->scaling, -x_scale, -y_scale);
    glUniform1i(shader->sprite, 0); // set texture manually

    const double Z_MUL = SPRITE_SPINE_PIXELS / SPRITE_HEIGHT_PIXELS;
    const double XY_MUL = SPRITE_BOWEL_PIXELS / SPRITE_HEIGHT_PIXELS;

    double x = 0.5 * (eye->x - eye->y);
    double y = -Z_MUL * eye->z - XY_MUL * (eye->x + eye->y);

    glUniform2f(shader->camera_position, x, y);
}

void shader_unbind(Shader* shader) {
    glUseProgram(0);
}

void shader_set_color(Shader* shader, int index, Color4f color) {
    glUniform3f(shader->colors[index], color.r, color.g, color.b);
}

void shader_set_tile_position(Shader* shader, Vector3i coordinate) {
    const double Z_MUL = SPRITE_SPINE_PIXELS / SPRITE_HEIGHT_PIXELS;
    const double XY_MUL = SPRITE_BOWEL_PIXELS / SPRITE_HEIGHT_PIXELS;

    double x = 0.5 * (coordinate.x - coordinate.y);
    double y = -Z_MUL * coordinate.z - XY_MUL * (coordinate.x + coordinate.y);

    glUniform2f(shader->texture_position, x - 0.5, y - 0.5);

    int camera_z = (int) shader->camera_vector3f.z; // forgive me
    double fraction = max_i(0, min_i(10, camera_z - coordinate.z)) / 10.0;
//    LOG_INFO_F("eye: %d, tile: %d, fraction: %6.03f", camera_z, coordinate.z, fraction);
    glUniform1f(shader->brightness, 1 - fraction);
}

void shader_free(Shader* shader) {
    glDeleteProgram(shader->ID);
    free(shader);
}

/// @see #new_Shader(const char*, const char* const char*)
typedef GLuint GLuint;

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

static inline GLuint create_shader(const char* shader_text, int type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &shader_text, NULL);
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

GLuint
create_program(const char* vertex_shader_text, const char* fragment_shader_text, const char* geometry_shader_text) {
    // create available shaders
    GLuint vertex_shader = create_shader(vertex_shader_text, GL_VERTEX_SHADER);
    if (!vertex_shader) {
        LOG_INFO("Failed to create vertex shader: aborting.");
        return 0;
    }

    GLuint fragment_shader = create_shader(fragment_shader_text, GL_FRAGMENT_SHADER);
    if (!fragment_shader) {
        LOG_INFO("Failed to create fragment shader: aborting.");
        return 0;
    }

    GLuint geometry_shader = 0;
    if (geometry_shader_text) {
        geometry_shader = create_shader(geometry_shader_text, GL_GEOMETRY_SHADER);
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
