if (CMAKE_USE_PTHREADS_INIT)
    set(UPDATE_ENGINE_IMPL UpdateEnginePT.c)
elseif (WIN32)
    set(UPDATE_ENGINE_IMPL UpdateEngineWin.c)
else ()
    set(UPDATE_ENGINE_IMPL UpdateEngineSingle.c)
endif ()

add_executable(
        YADF

        External/jasmine.c
        External/glew.c
        External/open-simplex-noise.c

        main.c
        DataStructures/Matrix4f.c
        DataStructures/Vector3f.c
        World/World.c
        Rendering/Mesh.c
        Rendering/Render.c
        DataStructures/StaticMonoAllocator.c
        Rendering/Camera.c
        Rendering/Phongshader.c
        World/${UPDATE_ENGINE_IMPL}
        Entities/Fluids.c
        World/WorldGenerator.c
        Entities/RockEntities.c
)

add_compile_definitions(GLEW_STATIC)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Shaders/phong.vert ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res/phong.vert COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Shaders/phong.frag ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res/phong.frag COPYONLY)

target_link_libraries(
        YADF
        Threads::Threads
        ${OPENGL_LIBRARIES}
        glfw
        ${CMAKE_THREAD_LIBS_INIT}
)