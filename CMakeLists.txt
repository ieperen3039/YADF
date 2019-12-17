cmake_minimum_required(VERSION 3.14)
project(YADF C)

set(CMAKE_C_STANDARD 99)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

find_package(GLEW QUIET)
if (NOT GLEW_FOUND) # if GLEW can't be found, download the sources and set GLEW_VERSION to its root directory.
    message(STATUS "OpenGL Extention Wrangler (GLEW) not found. Trying to find source")
    set(GLEW_VERSION glew-2.1.0)

    add_compile_definitions(GLEW_STATIC)
    add_subdirectory(../${GLEW_VERSION}/build/cmake glew)
    set(GLEW_LIBRARIES glew_s) # link with static library
endif()

find_package(glfw3 QUIET)
if (NOT glfw3_FOUND) # if glfw3 can't be found, download the sources and set GLFW_VERSION to its root directory.
    message(STATUS "GLFW not found. Trying to find source")
    set(GLFW_VERSION glfw-3.3)

    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(../${GLFW_VERSION} glfw)
endif ()

include_directories(include ${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})

add_executable(
        YADF
        src/main.c
        src/DataStructures/Matrix4f.c
        src/DataStructures/Vector3f.c
        src/World/World.c
        src/Rendering/Mesh.c
        src/DataStructures/jasmine.c
        src/Rendering/Render.c
        src/DataStructures/StaticMonoAllocator.c
        src/Rendering/Camera.c
        src/Rendering/Phongshader.c
        src/World/UpdateEngine.c
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/phong.vert ${CMAKE_CURRENT_BINARY_DIR}/res/phong.vert COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/phong.frag ${CMAKE_CURRENT_BINARY_DIR}/res/phong.frag COPYONLY)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(
        YADF
        ${OPENGL_LIBRARIES}
        ${GLEW_LIBRARIES}
        glfw
        ${CMAKE_THREAD_LIBS_INIT}
)

add_executable(
        unit_test EXCLUDE_FROM_ALL
        test/main.c
)

enable_testing()
add_test(
        NAME test_all
        COMMAND unit_test
)

