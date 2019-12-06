cmake_minimum_required(VERSION 3.15)
project(YADF C)

set(CMAKE_C_STANDARD 99)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 3.2 REQUIRED)
find_package(Threads REQUIRED)

include_directories(include ${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})

add_executable(
        YADF
        src/main.c
        src/Rendering/Shader.h src/Rendering/Shader.c
        src/DataStructures/List.h
        src/DataStructures/Matrix4f.c
        src/DataStructures/Vector3f.c
        src/world.h src/world.c
        src/Rendering/Mesh.c src/Rendering/Mesh.h
        src/DataStructures/jasmine.h src/DataStructures/jasmine.c
        src/Rendering/Render.h src/Rendering/Render.c
        src/DataStructures/StaticMonoAllocator.c src/DataStructures/StaticMonoAllocator.h
        src/Materials.h src/Rendering/Camera.c src/Rendering/Camera.h)

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
        unit_tests
        test/List_test.c
)