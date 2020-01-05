//
// Created by ieperen3039 on 12-12-19.
//

#include "Shader.h"
#include "ShaderControl.h"
#include "../YADFEngine/Tools.h"

#include <malloc.h>
#include <assert.h>

#define SHADER_MAX_NUM_LIGHTS 16
#define SHADER_MAX_NUM_MATERIALS 8 // materials per entity

static const Vector3fc AMBIENT_LIGHT = {0.15f, 0.15f, 0.15f};

struct _Shader {
    GLuint ID;
    GLuint viewProjectionMatrix; // matrix4f
    GLuint origin; // vec3f
    GLuint rotation; // byte
    GLuint cameraPosition; // vec3f
    GLuint ambientLight; // vec3f

    int nextLightIndex;
    struct SLight {
        GLuint color;
        GLuint mPosition;
        GLuint intensity;
    } lights[SHADER_MAX_NUM_LIGHTS];

    GLuint colors[SHADER_MAX_NUM_MATERIALS];

    Color4f material_colors[ENUM_SIZE(Material)];
};

static GLuint
create_program(const char* vertex_shader_text, const char* fragment_shader_text, const char* geometry_shader_text);

Shader* shader_new(Color4f* material_properties) {
    char* vertex_shader = tool_read_file("res/flat_shader.vert", NULL);
    char* fragment_shader = tool_read_file("res/flat_shader.frag", NULL);
    if (vertex_shader == NULL || fragment_shader == NULL) return NULL;

    GLuint shader = create_program(vertex_shader, fragment_shader, NULL);
    if (!shader) return NULL;

    Shader* p = malloc(sizeof(Shader));
    p->ID = shader;
    p->viewProjectionMatrix = glGetUniformLocation(shader, "viewProjectionMatrix");
    p->origin = glGetUniformLocation(shader, "origin");
    p->rotation = glGetUniformLocation(shader, "rotation");
    p->ambientLight = glGetUniformLocation(shader, "ambientLight");
    p->cameraPosition = glGetUniformLocation(shader, "cameraPosition");

    for (int i = 0; i < SHADER_MAX_NUM_LIGHTS; ++i) {
        char uniform_name[25];
        int curr = snprintf(uniform_name, 12, "lights[%d]", i);
        char* field = uniform_name + curr;

        strcpy(field, ".color");
        p->lights[i].color = glGetUniformLocation(shader, uniform_name);
        strcpy(field, ".mPosition");
        p->lights[i].mPosition = glGetUniformLocation(shader, uniform_name);
        strcpy(field, ".intensity");
        p->lights[i].intensity = glGetUniformLocation(shader, uniform_name);
    }

    for (int i = 0; i < SHADER_MAX_NUM_MATERIALS; ++i) {
        char uniform_name[15];
        snprintf(uniform_name, 15, "materials[%d]", i);
        p->colors[i] = glGetUniformLocation(shader, uniform_name);
    }

    for (int i = 0; i < MaterialSize; ++i) {
        p->material_colors[i] = material_properties[i];
    }

    return p;
}

void shader_bind(Shader* shader, Vector3fc* camera_eye) {
    glUseProgram(shader->ID);
    glUniform3f(shader->ambientLight, AMBIENT_LIGHT.x, AMBIENT_LIGHT.y, AMBIENT_LIGHT.z);
    glUniform3f(shader->cameraPosition, camera_eye->x, camera_eye->y, camera_eye->z);

    // set all lights to zero
    for (int i = 0; i < SHADER_MAX_NUM_LIGHTS; ++i) {
        glUniform1f(shader->lights[i].intensity, 0);
    }
    shader->nextLightIndex = 0;
}

void shader_unbind(Shader* shader) {
    glUseProgram(0);
}

void shader_add_light(Shader* shader, Vector3fc position, Color4f color, float intensity, bool infinite) {
    int lightNumber = shader->nextLightIndex++;
    assert(lightNumber < SHADER_MAX_NUM_LIGHTS);
    assert(color.a > 0);
    struct SLight li = shader->lights[lightNumber];

    glUniform4f(li.mPosition, position.x, position.y, position.z, infinite ? 0.0f : 1.0f);
    glUniform3f(li.color, color.r / color.a, color.g / color.a, color.b / color.a);
    glUniform1f(li.intensity, intensity);
}

void shader_set_material(Shader* shader, int index, Material material) {
    assert(index < SHADER_MAX_NUM_MATERIALS);

    Color4f color = (material < 0) ? COLOR_BLACK : shader->material_colors[material];

    glUniform3f(shader->colors[index], color.r, color.g, color.b);
}

void shader_set_tile_position(Shader* shader, Vector3i coordinate) {
    // real position is twice the coordinate; tiles are 2 in size
    glUniform3i(shader->origin, 2 * coordinate.x, 2 * coordinate.y, 2 * coordinate.z);
}

void shader_set_view_projection_matrix(Shader* shader, Matrix4f* matrix) {
    glUniformMatrix4fv(shader->viewProjectionMatrix, 1, false, matrix_as_array(matrix));
}

void shader_free(Shader* shader) {
    glDeleteProgram(shader->ID);
    free(shader);
}

GLuint shader_id(Shader* shader) { return shader->ID; }

void shader_set_tile_rotation(Shader* shader, char rotation) {
    glUniform1i(shader->rotation, rotation);
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
