#include "DataStructures/Matrix4f.h"
#include "DataStructures/List.h"
#include "global.h"
#include "Tools.h"
#include "Rendering/Phongshader.h"
#include "Rendering/Render.h"
#include "World/World.h"

#include <stdio.h>
#include <glfw3.h>

volatile bool hasGLError = 0;

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

int main(int argc, char** argv) {
    LOG_INFO("Starting GLFW");
    glfwSetErrorCallback(glfwErrorCallback);

    int success = glfwInit();
    if (!success) {
        LOG_ERROR("Failed GLFW init");
        return EXIT_FAILURE;
    }
    int window_width = 800;
    int window_height = 600;
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "View", NULL, NULL);

    if (!window) {
        LOG_ERROR("Failed to create window");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    LOG_INFO("Starting OpenGL");
    GLenum error = glewInit();
    if (error != GLEW_OK) {
        LOG_ERROR_F("Failed to initialize GLEW: %s", glewGetErrorString(error));
        return EXIT_FAILURE;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(glErrorCallback, NULL);

    glClearColor(1.0f, 0.0f, 1.0f, 0.0f); // magenta
//    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // black
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    LOG_INFO_F("Started GLFW with OpenGL version %s", glGetString(GL_VERSION));

    LOG_INFO("Initializing Shader");
    Phongshader* shader = phong_create();
    if (!shader) return(EXIT_FAILURE);

    LOG_INFO("Starting world");
    World* world = world_new(1000);
    if (!world) return(EXIT_FAILURE);
    Camera* camera = camera_new(&VECTOR_ZERO);

    LOG_INFO("Setup done!");

    while (!glfwWindowShouldClose(window) && !hasGLError) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwGetWindowSize(window, &window_width, &window_height);
        glViewport(0, 0, window_width, window_height);
        
        render_frame(shader, world, camera, (float) window_width / (float) window_height);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    if (hasGLError) {
        LOG_INFO("Closing due to a GL error...");

    } else {
        LOG_INFO("Closing...");
    }

    phong_free(shader);
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}