#include "global.h"
#include "Rendering/Phongshader.h"
#include "Rendering/Render.h"
#include "World/UpdateEngine.h"
#include "World/WorldGenerator.h"

#include <stdio.h>
#include <GLFW/glfw3.h>
#include <GL/glew.h>

// sleep function
#if defined(WIN32)

    #include <time.h>
    #include <windows.h>

    #define wait_for(time_ms) Sleep((time_ms))

#elif defined(LINUX)
    #include <unistd.h>
    #define wait_for(time_ms) usleep((time_ms) * 1000)

#else
    #define wait_for(time_ms) ; // don't wait at all
#endif

#define GAME_FPS 10

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
    LOG_INFO("Reading parameters...");

    LOG_INFO("Starting GLFW...");
    glfwSetErrorCallback(glfwErrorCallback);

    int success = glfwInit();
    if (!success) {
        LOG_ERROR("Failed GLFW init");
        return EXIT_FAILURE;
    }
    int window_width = 800;
    int window_height = 600;
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Yet Another Dwarf Fortress", NULL, NULL);

    if (!window) {
        LOG_ERROR("Failed to create window");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    LOG_INFO("Starting OpenGL...");
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

    LOG_INFO("Initializing Shader...");
    Phongshader* shader = phong_create();
    if (!shader) return EXIT_FAILURE;

    Camera* camera = camera_new(&VECTOR_ZERO);

    LOG_INFO("Creating Worker Threads");
    UpdateWorkerPool* workers = update_workers_new();

    LOG_INFO("Starting world...");
    World* world = world_new(1000);
    if (!world) return EXIT_FAILURE;

    // initialize starting area
    generator_generate_default(world, (BoundingBox) {-100, -100, -100, 100, 100, 100});

    LOG_INFO("Setup done!");
    UpdateCycle game_time = 0;
    time_t loop_time = clock();

    while (!glfwWindowShouldClose(window) && !hasGLError) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwGetWindowSize(window, &window_width, &window_height);
        glViewport(0, 0, window_width, window_height);

        time_t update_start = clock();
        update_world_tick(world, game_time, workers);

        time_t render_start = clock();
        render_frame(shader, world, camera, (float) window_width / (float) window_height);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        time_t callback_start = clock();
        glfwPollEvents();

        time_t end_time = clock();
        LOG_INFO_F(
                "Loop %4d: [update : %5ld ms][render : %5ld ms][callbacks : %5ld ms][total: %6ld ms]",
                game_time, render_start - update_start, callback_start - render_start, end_time - callback_start,
                end_time - loop_time
        );

        long wait_time = (long) (((double) 1000 / GAME_FPS) - (double) (end_time - loop_time));
        if (wait_time > 0) {
            wait_for(wait_time);
            loop_time += ((double) 1000 / GAME_FPS);

        } else {
            loop_time = clock();
        }

        game_time++;
    }

    if (hasGLError) {
        LOG_INFO("Closing due to a GL error...");

    } else {
        LOG_INFO("Closing...");
    }

    update_workers_free(workers);
    phong_free(shader);
    world_free(world);
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}