//
// Created by s152717 on 24-12-2019.
//

#ifndef YADF_TESTMAP_H
#define YADF_TESTMAP_H

#include "../src/YADFEngine/DataStructures/Map.h"
#include "../src/YADFEngine/DataStructures/StaticMonoAllocator.h"

AllocatorSM* alloc_map;

void test_map_get_list(void* target, void* null) {
    list_init(target, sizeof(TYPE), 0);
}

ElementMap* new_map() {
    ElementMap* list = allocator_sm_alloc(alloc_map);
    return map_init(list, sizeof(TYPE));
}

void test_map_add_get(CuTest* tc) {
    ElementMap* map = new_map();
    map_insert(map, 2, &elements[2]);
    map_insert(map, 3, &elements[3]);
    map_insert(map, 4, &elements[4]);

    CuAssertDblEquals(tc, elements[4], *(TYPE*) map_get(map, 4), 0);
    CuAssertDblEquals(tc, elements[2], *(TYPE*) map_get(map, 2), 0);
    CuAssertDblEquals(tc, elements[3], *(TYPE*) map_get(map, 3), 0);

    // double elements
    map_insert(map, 10, &elements[2]);
    CuAssertDblEquals(tc, elements[2], *(TYPE*) map_get(map, 10), 0);

    // overwrite key
    map_insert(map, 2, &elements[7]);
    CuAssertDblEquals(tc, elements[7], *(TYPE*) map_get(map, 2), 0);
}

void test_map_remove_contains(CuTest* tc) {
    ElementMap* map = new_map();
    map_insert(map, 2, &elements[2]);
    map_insert(map, 3, &elements[3]);
    map_insert(map, 4, &elements[4]);

    CuAssertTrue(tc, map_contains(map, 3));

    bool contained = map_remove(map, 3);
    CuAssertTrue(tc, contained);

    CuAssertPtrEquals(tc, NULL, map_get(map, 3));
    CuAssertTrue(tc, !map_contains(map, 3));
    CuAssertDblEquals(tc, elements[4], *(TYPE*) map_get(map, 4), 0);
    CuAssertDblEquals(tc, elements[2], *(TYPE*) map_get(map, 2), 0);

    bool contained2 = map_remove(map, 5);
    CuAssertTrue(tc, !contained2);
    // also queries element that has never been mentioned before
    CuAssertPtrEquals(tc, NULL, map_get(map, 5));
    CuAssertDblEquals(tc, elements[4], *(TYPE*) map_get(map, 4), 0);
    CuAssertDblEquals(tc, elements[2], *(TYPE*) map_get(map, 2), 0);
}

void test_map_compute_absent(CuTest* tc) {
    ElementMap* map = allocator_sm_alloc(alloc_map);
    map_init(map, sizeof(List));

    List* list1 = map_compute_if_absent(map, 10, test_map_get_list, NULL);
    List* list2 = map_compute_if_absent(map, 20, test_map_get_list, NULL);
    List* list3 = map_compute_if_absent(map, 10, test_map_get_list, NULL);

    CuAssertPtrEquals(tc, list1, list3);
    CuAssertTrue(tc, list1 != list2);

    list_add(list1, &elements[0]);
    list_add(list1, &elements[1]);

    CuAssertIntEquals(tc, 0, list_get_size(list2));
    list_add(list2, &elements[3]);

    CuAssertDblEquals(tc, elements[1], *(TYPE*) list_get(list1, 1), 0);
    CuAssertDblEquals(tc, elements[3], *(TYPE*) list_get(list2, 0), 0);
}

void test_map_overload(CuTest* tc) {
    ElementMap* map = map_init_sized(malloc(sizeof(ElementMap)), sizeof(TYPE), 4, 0.5f);

    map_insert(map, 0, &elements[0]);
    map_insert(map, 1, &elements[1]);
    map_insert(map, 2, &elements[2]);
    map_insert(map, 3, &elements[3]);
    map_insert(map, 4, &elements[4]);
    map_insert(map, 5, &elements[5]);
    map_insert(map, 6, &elements[6]);
    map_insert(map, 7, &elements[7]);
    map_insert(map, 8, &elements[8]);
    map_insert(map, 9, &elements[9]);
    map_insert(map, 1000, &elements[0]);
    map_insert(map, 1001, &elements[1]);
    map_insert(map, 1002, &elements[2]);
    map_insert(map, 1003, &elements[3]);
    map_insert(map, 1004, &elements[4]);
    map_insert(map, 1005, &elements[5]);
    map_insert(map, 1006, &elements[6]);
    map_insert(map, 1007, &elements[7]);
    map_insert(map, 1008, &elements[8]);
    map_insert(map, 1009, &elements[9]);
    map_insert(map, 20000, &elements[0]);
    map_insert(map, 20001, &elements[1]);
    map_insert(map, 20002, &elements[2]);
    map_insert(map, 20003, &elements[3]);
    map_insert(map, 20004, &elements[4]);
    map_insert(map, 20005, &elements[5]);
    map_insert(map, 20006, &elements[6]);
    map_insert(map, 20007, &elements[7]);
    map_insert(map, 20008, &elements[8]);
    map_insert(map, 20009, &elements[9]);

    CuAssertIntEquals(tc, 30, map_get_size(map));
    CuAssertDblEquals(tc, elements[3], *(TYPE*) map_get(map, 3), 0);
    CuAssertDblEquals(tc, elements[4], *(TYPE*) map_get(map, 1004), 0);
    CuAssertDblEquals(tc, elements[5], *(TYPE*) map_get(map, 20005), 0);
    CuAssertDblEquals(tc, elements[6], *(TYPE*) map_get(map, 6), 0);
    CuAssertDblEquals(tc, elements[7], *(TYPE*) map_get(map, 1007), 0);
    CuAssertDblEquals(tc, elements[8], *(TYPE*) map_get(map, 20008), 0);

    bool contained1 = map_remove(map, 6);
    CuAssertTrue(tc, contained1);
    CuAssertTrue(tc, !map_contains(map, 6));

    bool contained2 = map_remove(map, 3);
    CuAssertTrue(tc, contained2);
    CuAssertTrue(tc, !map_contains(map, 3));

    CuAssertPtrEquals(tc, NULL, map_get(map, 3));
    CuAssertDblEquals(tc, elements[4], *(TYPE*) map_get(map, 1004), 0);
    CuAssertDblEquals(tc, elements[5], *(TYPE*) map_get(map, 20005), 0);
    CuAssertPtrEquals(tc, NULL, map_get(map, 6));
    CuAssertDblEquals(tc, elements[7], *(TYPE*) map_get(map, 1007), 0);
    CuAssertDblEquals(tc, elements[8], *(TYPE*) map_get(map, 20008), 0);

    map_free(map);
    free(map);
}

void test_map_empty(CuTest* tc) {
    CuAssertIntEquals(tc, map_get_size(&MAP_EMPTY), 0);
}

CuSuite* map_suite(void) {
    CuSuite* suite = CuSuiteNew();
    alloc_map = allocator_sm_new(sizeof(ElementMap));

    SUITE_ADD_TEST(suite, test_map_add_get);
    SUITE_ADD_TEST(suite, test_map_remove_contains);
    SUITE_ADD_TEST(suite, test_map_compute_absent);
    SUITE_ADD_TEST(suite, test_map_overload);

    return suite;
}

void map_cleanup() {
    allocator_sm_free(alloc_map);
}


#endif //YADF_TESTMAP_H
