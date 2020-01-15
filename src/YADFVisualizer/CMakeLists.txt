set(DIRECTORY "../..")
set(ENGINE_DIRECTORY ${DIRECTORY}/src/YADFEngine)

find_package(OpenGL REQUIRED)

find_package(glfw3 QUIET)
if (NOT glfw3_FOUND) # if glfw3 can't be found, download the sources and set GLFW_VERSION to its root directory.
    set(GLFW_VERSION "glfw-3.3")
    message(STATUS "GLFW not found. Using source in ${GLFW_VERSION}")

    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(${DIRECTORY}/../${GLFW_VERSION} glfw)
endif ()

include_directories(${OPENGL_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})

configure_file(../YADFVisualizer/Shaders/tile_shader.vert ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res/tile_shader.vert COPYONLY)
configure_file(../YADFVisualizer/Shaders/tile_shader.frag ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res/tile_shader.frag COPYONLY)

add_executable(
        YADFVisualizer
        main.c

        External/nanovg/nanovg.c
        External/glew.c
        External/cJSON.c

        Sprite.c
        Shader.c
        Window.c
        RenderFrame.c
)

target_compile_definitions(YADFVisualizer PUBLIC GLEW_STATIC)

target_link_libraries(
        YADFVisualizer
        YADFEngine
        ${OPENGL_LIBRARIES}
        glfw
)