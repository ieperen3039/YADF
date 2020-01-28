add_executable(
        unit_test EXCLUDE_FROM_ALL
        test_main.c
        CuTest.c

        ../src/YADFEngine/DataStructures/StaticMonoAllocator.c
        ../src/YADFEngine/Entities/EntityRead.c
        ../src/YADFEngine/Entities/EntityWrite.c
        ../src/YADFEngine/Entities/EntityClass.c
)

target_link_libraries(
        unit_test
        m
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 --coverage -fno-inline -fno-inline-small-functions -fno-default-inline --branch-count-reg")

add_test(
        NAME unit_test
        COMMAND unit_test
)

set_tests_properties(unit_test PROPERTIES TIMEOUT 10)