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
    Phongshader* shader = phong_create();
    if (!shader) return(EXIT_FAILURE);

    LOG_INFO("Starting world");
    World* world = world_new(1000);
    if (!world) return(EXIT_FAILURE);
    Camera* camera = camera_new(&VECTOR_X);

    LOG_INFO("Setup done!");

    Matrix4f t = camera_get_transform(camera);
    int l = matrix_to_string(&t, NULL, 0);
    char str[l];
    matrix_to_string(&t, str, l);
    LOG_INFO_F("Camera transform: %s", str);

    while (!glfwWindowShouldClose(window) && !hasGLError) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, window_width, window_height);

        render_frame(shader, world, camera, 0);

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