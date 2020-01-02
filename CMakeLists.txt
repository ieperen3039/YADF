cmake_minimum_required(VERSION 3.14)
project(YADF C)

set(CMAKE_C_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

find_package(glfw3 QUIET)
if (NOT glfw3_FOUND) # if glfw3 can't be found, download the sources and set GLFW_VERSION to its root directory.
    set(GLFW_VERSION glfw-3.3)
    message(STATUS "GLFW not found. Using source in ../${GLFW_VERSION}")

    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(../${GLFW_VERSION} glfw)
endif ()

include_directories(include ${OPENGL_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin)

add_subdirectory(src)

enable_testing()
add_subdirectory(test)

