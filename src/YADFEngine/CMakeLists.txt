

if (YADF_MULTITHREADING)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    set(UPDATE_ENGINE_IMPL UpdateEngine.c)
else ()
    set(UPDATE_ENGINE_IMPL UpdateEngineSingle.c)
endif ()

add_library(
        YADFEngine SHARED

        External/open-simplex-noise.c
        External/cJSON.c

        YADFEngine.c
        DataStructures/Matrix4f.c
        DataStructures/Vector3f.c
        DataStructures/StaticMonoAllocator.c
        UpdateEngine/${UPDATE_ENGINE_IMPL}
        Entities/Fluids.c
        World/WorldGenerator.c
        Entities/EntityClass.c
        Entities/EntityUpdateStatic.c
        Entities/EntityWrite.c
        World/World.c
)
target_compile_definitions(YADFEngine PRIVATE YADF_BUILD)


if (YADF_MULTITHREADING)
target_link_libraries(
        YADFEngine
        Threads::Threads
        ${CMAKE_THREAD_LIBS_INIT}
)
endif()

set_target_properties(YADFEngine PROPERTIES VERSION ${PROJECT_VERSION})

set_target_properties(YADFEngine PROPERTIES
        PUBLIC_HEADER
        ${HEADER_DIRECTORY}/global.h
        )