//
// Created by s152717 on 4-1-2020.
//
#include <YADFEngine.h>

#include "Window.h"
#include "Shader.h"
#include "RenderFrame.h"
#include "Sprite.h"

#include "External/cJSON.h"

struct _Visualizer {
    GLFWwindow* window;
    Shader* shader;
    Vector3f focus;
    Color4f material_map[ENUM_SIZE(Material)];
    Sprite* entity_sprites[ENUM_SIZE(EntityClass)];
    volatile bool hasGLError;
};

static void glfwErrorCallback(int error, const char* description) {
    LOG_ERROR_F("GLFW Error: %s", description);
}

static void GLAPIENTRY glErrorCallback(
        GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
        const void* userParam
) {
    if (type == GL_DEBUG_TYPE_ERROR) {
        LOG_ERROR(message);

        Visualizer* v = (Visualizer*) userParam;
        v->hasGLError = true;

    } else {
//        LOG_INFO(message);
    }
}

ErrorCode json_read_config(const char* file, Color4f* material_colors);

Visualizer* visualizer_create_window() {
    glfwSetErrorCallback(glfwErrorCallback);

    int success = glfwInit();
    if (!success) {
        LOG_ERROR("Failed GLFW init");
        return NULL;
    }

    int window_width = 1000;
    int window_height = 600;
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Yet Another Dwarf Fortress", NULL, NULL);

    if (!window) {
        LOG_ERROR("Failed to create window");
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    GLenum error = glewInit();
    if (error != GLEW_OK) {
        LOG_ERROR_F("Failed to initialize GLEW: %s", glewGetErrorString(error));
        return NULL;
    }

    Visualizer* vis = malloc(sizeof(Visualizer));
    vis->window = window;
    vis->hasGLError = false;

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(glErrorCallback, vis);

    glClearColor(1.0f, 0.0f, 1.0f, 0.0f); // magenta
//    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // black
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

    LOG_INFO_F("Started OpenGL version %s", glGetString(GL_VERSION));

    vis->shader = shader_new();
    if (!vis->shader) return NULL;

    vis->focus = VECTOR_ZERO;
    json_read_config("config.json", vis->material_map);

    sprite_init();

    for (int i = 0; i < EntityClassSize; ++i) {
        const struct EntityClassData* entity_class = yadf_get_entity_class(EntityClassValues[i]);
        char filename[64];
        snprintf(filename, 64, "res/sprites/%s.png", entity_class->name);
        vis->entity_sprites[i] = sprite_new(filename);
    }

    return vis;
}

bool visualizer_is_closed(Visualizer* vis) {
    return vis->hasGLError || glfwWindowShouldClose(vis->window);
}

bool visualizer_has_error(Visualizer* vis) {
    return vis->hasGLError;
}

void visualizer_free(Visualizer* vis) {
    shader_free(vis->shader);
    glfwDestroyWindow(vis->window);
    glfwTerminate();
    free(vis);
}

void visualizer_callbacks(Visualizer* vis) {
    glfwPollEvents();
}

void visualizer_draw_frame(Visualizer* vis, World* world) {
    int window_width;
    int window_height;
    glfwGetWindowSize(vis->window, &window_width, &window_height);

    // TODO: GL_COLOR_BUFFER_BIT can be removed
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glViewport(0, 0, window_width, window_height);

    // TODO implement camera movement
    const Sprite** sprites = (const Sprite**) vis->entity_sprites;
    render_frame(vis->shader, world, &vis->focus, vis->material_map, sprites, window_width, window_height);

    /* Swap front and back buffers */
    glfwSwapBuffers(vis->window);
}

void json_read_material(Color4f* material_colors, cJSON* elt) {
    while (elt != NULL) {
        const char* name = elt->string;

        Color4f color;
        if (cJSON_IsArray(elt)) {
            unsigned char r = cJSON_GetArrayItem(elt, 0)->valueint & 0xFF;
            unsigned char g = cJSON_GetArrayItem(elt, 1)->valueint & 0xFF;
            unsigned char b = cJSON_GetArrayItem(elt, 2)->valueint & 0xFF;

            color = (Color4f) {r / 255.0f, g / 255.0f, b / 255.0f, 1};

        } else {
            const char* hex = cJSON_GetStringValue(elt);
            if (!hex) {
                LOG_ERROR_F("Value of element %s is not an array, nor a string hex", name);
                goto next;
            }

            if (hex[0] == '#') hex++; // skip #
            unsigned int color_hex = strtol(hex, NULL, 16);
            unsigned char r = (color_hex >> 0) & 0xFF;
            unsigned char g = (color_hex >> 8) & 0xFF;
            unsigned char b = (color_hex >> 16) & 0xFF;

            color = (Color4f) {r / 255.0f, g / 255.0f, b / 255.0f, 1};
        }

        for (int i = 0; i < MaterialSize; ++i) {
            Material mat = MaterialValues[i];

            if (tool_case_insensitive_strcmp(material_name(mat), name) == 0) {
                material_colors[i] = color;
                goto next;
            }
        }
        LOG_ERROR_F("Could not match %s with any of the known materials", name);

        next:
        elt = elt->next;
    }
}

ErrorCode json_read_config(const char* file, Color4f* material_colors) {
    assert(material_colors != NULL);

    int file_length;
    char* file_string = tool_read_file(file, &file_length);
    if (!file_string) return ERROR_IO;

    tool_strip_char(file_string, file_length, ' ');

    cJSON* json = cJSON_Parse(file_string);
    if (!json) goto error_parsing;

    cJSON* j_material = cJSON_GetObjectItem(json, "materials");
    if (!j_material) goto error_missing_field;

    json_read_material(material_colors, j_material->child);

    cJSON_Delete(json);

    return ERROR_NONE;

    error_parsing:;
    const char* error_point = cJSON_GetErrorPtr();
    char msg[21];
    memcpy(msg, error_point, 20);
    msg[20] = '\0';

    LOG_ERROR_F("Failed to read json file. Error occurred while reading '%s'", msg);
    return ERROR_JSON;

    error_missing_field:
    LOG_ERROR("json file did not contain a 'materials' field");
    return ERROR_JSON;
}