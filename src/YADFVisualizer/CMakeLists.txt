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

include_directories(${DIRECTORY}/include ${OPENGL_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})

configure_file(../YADFVisualizer/Shaders/flat_shader.vert ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res/flat_shader.vert COPYONLY)
configure_file(../YADFVisualizer/Shaders/flat_shader.frag ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res/flat_shader.frag COPYONLY)

add_executable(
        YADFVisualizer
        main.c

        External/glew.c
        ${ENGINE_DIRECTORY}/External/cJSON.c

        ${ENGINE_DIRECTORY}/Tools.c
        ${ENGINE_DIRECTORY}/World/World.c
        ${ENGINE_DIRECTORY}/DataStructures/Matrix4f.c
        ${ENGINE_DIRECTORY}/DataStructures/Vector3f.c
        ${ENGINE_DIRECTORY}/DataStructures/StaticMonoAllocator.c

        Mesh.c
        Shader.c
        Window.c
        Visual.c
        Camera.c
        MaterialProperties.c
)

target_compile_definitions(YADFVisualizer PUBLIC GLEW_STATIC)

target_link_libraries(
        YADFVisualizer
        YADFEngine
        ${OPENGL_LIBRARIES}
        glfw
)