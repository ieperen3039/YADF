//
// Created by s152717 on 4-1-2020.
//

#include "Window.h"
#include "Shader.h"
#include "Camera.h"
#include "RenderFrame.h"
#include "MaterialProperties.h"

struct _Visualizer {
    GLFWwindow* window;
    Shader* shader;
    Camera* camera;
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

Visualizer* visualizer_create_window() {
    glfwSetErrorCallback(glfwErrorCallback);

    int success = glfwInit();
    if (!success) {
        LOG_ERROR("Failed GLFW init");
        return NULL;
    }

    int window_width = 800;
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
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    LOG_INFO_F("Started OpenGL version %s", glGetString(GL_VERSION));

    Color4f mats[MaterialSize];
    material_read_json("config.json", mats);

    vis->shader = shader_new(mats);
    if (!vis->shader) return NULL;

    vis->camera = camera_new(&VECTOR_ZERO);

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

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, window_width, window_height);

    float aspect_ratio = (float) window_width / (float) window_height;
    render_frame(vis->shader, world, vis->camera, aspect_ratio);

    /* Swap front and back buffers */
    glfwSwapBuffers(vis->window);
}
