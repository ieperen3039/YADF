#include <stdio.h>
#include "DataStructures/Matrix4f.h"
#include "DataStructures/List.h"
#include "global.h"
#include "Tools.c"
#include "Rendering/Shader.h"
#include "Rendering/Render.h"

volatile bool hasGLError = 0;

volatile int window_width = 800;
volatile int window_height = 600;

static void glfwErrorCallback(int error, const char* description) {
    LOG_ERROR_F("GLFW Error: %s", description);
}

static void GLAPIENTRY glErrorCallback(
        GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
        const void* userParam
) {
    if (type == GL_DEBUG_TYPE_ERROR) {
        LOG_ERROR(message);
        hasGLError = true;

    } else {
//        LOG_INFO(message);
    }
}

static void glfwResizeCallback(GLFWwindow* w, int width, int height) {
    window_width = width;
    window_height = height;
}

int main(int argc, char** argv) {
    LOG_INFO("Starting GLFW");
    glfwSetErrorCallback(glfwErrorCallback);

    int success = glfwInit();
    if (!success) {
        LOG_ERROR("Failed GLFW init");
        return EXIT_FAILURE;
    }
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "View", NULL, NULL);

    if (!window) {
        LOG_ERROR("Failed to create window");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetWindowSizeCallback(window, glfwResizeCallback);

    LOG_INFO("Starting OpenGL");
    GLenum error = glewInit();
    if (error != GLEW_OK) {
        LOG_ERROR_F("Failed to initialize GLEW: %s", glewGetErrorString(error));
        return EXIT_FAILURE;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(glErrorCallback, NULL);

    glClearColor(1.0f, 0.0f, 1.0f, 0.0f); // magenta
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    LOG_INFO_F("Started GLFW with OpenGL version %s", glGetString(GL_VERSION));

    LOG_INFO("Initializing Shader");
    char* vertex_shader = readFile("res/phong.vert", NULL);
    char* fragment_shader = readFile("res/phong.frag", NULL);
    ShaderID shader = shader_create(vertex_shader, fragment_shader, NULL);

    LOG_INFO("Starting world");
    World* world = world_new(1000);

    LOG_INFO("Setup done!");

    while (!glfwWindowShouldClose(window) && !hasGLError) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, window_width, window_height);

        render_frame(shader, world, camera_new(&VECTOR_X), 0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    if (hasGLError) {
        LOG_INFO("Closing due to a GL error\n");
    }

    shader_free(shader);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}