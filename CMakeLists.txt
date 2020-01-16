cmake_minimum_required(VERSION 3.14)
project(YADF VERSION 0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH})
set(YADF_MULTITHREADING TRUE)

include_directories(include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/out)
configure_file(config.json ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json COPYONLY)

add_subdirectory(src/YADFEngine)
add_subdirectory(src/YADFVisualizer)

enable_testing()
add_subdirectory(test)

