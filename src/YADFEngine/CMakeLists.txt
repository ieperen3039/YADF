set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if (CMAKE_USE_PTHREADS_INIT)
    set(UPDATE_ENGINE_IMPL UpdateEnginePT.c)
elseif (WIN32)
    set(UPDATE_ENGINE_IMPL UpdateEngineWin.c)
else ()
    set(UPDATE_ENGINE_IMPL UpdateEngineSingle.c)
endif ()

set(HEADER_DIRECTORY "../../include")

include_directories(${HEADER_DIRECTORY})

add_library(
        YADFEngine SHARED

        External/open-simplex-noise.c
        External/cJSON.c

        API.c
        DataStructures/Matrix4f.c
        DataStructures/Vector3f.c
        DataStructures/StaticMonoAllocator.c
        World/${UPDATE_ENGINE_IMPL}
        Entities/Fluids.c
        World/WorldGenerator.c
        Entities/EntityClass.c
        Entities/EntityImplStatic.c
        Tools.c
        Entities/Entity.c
        World/World.c
)
target_compile_definitions(YADFEngine PRIVATE YADF_BUILD)

target_link_libraries(
        YADFEngine
        Threads::Threads
        ${CMAKE_THREAD_LIBS_INIT}
)

set_target_properties(YADFEngine PROPERTIES VERSION ${PROJECT_VERSION})

set_target_properties(YADFEngine PROPERTIES
        PUBLIC_HEADER
        ${HEADER_DIRECTORY}/global.h
)