//
// Created by s152717 on 23-12-2019.
//

#ifndef YADF_TESTLIST_H
#define YADF_TESTLIST_H

#include "CuTest.h"

#include "../src/DataStructures/List.h"
#include "../src/DataStructures/StaticMonoAllocator.h"
#include <limits.h>

AllocatorSM* alloc_list;

List* new_list() {
    List* list = allocator_sm_alloc(alloc_list);
    assert(list != NULL);
    return list_init(list, sizeof(TYPE), 16);
}

void test_list_add_get(CuTest* tc) {
    List* list = new_list();
    list_add(list, &elements[0]);
    list_add(list, &elements[1]);

    CuAssertDblEquals(tc, elements[0], *((TYPE*) list_get(list, 0)), 0);
    CuAssertDblEquals(tc, elements[1], *((TYPE*) list_get(list, 1)), 0);

    list_add(list, &elements[2]);
    CuAssertDblEquals(tc, elements[2], *((TYPE*) list_get(list, 2)), 0);

    CuAssertIntEquals(tc, 3, list_get_size(list));

}

void test_list_set(CuTest* tc) {
    List* list = new_list();
    list_add(list, &elements[0]);
    list_add(list, &elements[1]);
    list_add(list, &elements[2]);
    list_add(list, &elements[3]);

    ErrorCode ec1 = list_set(list, 2, &elements[5]);
    CuAssertIntEquals(tc, ERROR_NONE, ec1);
    CuAssertDblEquals(tc, elements[5], *((TYPE*) list_get(list, 2)), 0);

    ErrorCode ec2 = list_set(list, 2, &elements[7]);
    CuAssertIntEquals(tc, ERROR_NONE, ec2);
    CuAssertDblEquals(tc, elements[7], *((TYPE*) list_get(list, 2)), 0);

    ErrorCode ec3 = list_set(list, 4, &elements[8]);
    CuAssertIntEquals(tc, ERROR_OUT_OF_BOUNDS, ec3);

    CuAssertPtrEquals(tc, NULL, list_get_checked(list, 4));
    CuAssertIntEquals(tc, 4, list_get_size(list));

    CuAssertPtrEquals(tc, NULL, list_get_checked(list, -1));
    CuAssertPtrEquals(tc, NULL, list_get_checked(list, INT_MAX));
    CuAssertPtrEquals(tc, NULL, list_get_checked(list, INT_MIN));
}

void test_list_get_checked(CuTest* tc) {
    List* list = new_list();
    list_add(list, &elements[0]);
    list_add(list, &elements[1]);

    CuAssertDblEquals(tc, elements[1], *((TYPE*) list_get_checked(list, 1)), 0);
    CuAssertPtrEquals(tc, NULL, list_get_checked(list, 2));
    CuAssertPtrEquals(tc, NULL, list_get_checked(list, 3));
}

void test_list_delete_index(CuTest* tc) {
    List* list = new_list();
    list_add(list, &elements[0]);
    list_add(list, &elements[1]);
    list_add(list, &elements[2]);
    list_add(list, &elements[3]);
    ErrorCode ec = list_delete_index(list, 1);

    CuAssertIntEquals(tc, ERROR_NONE, ec);
    CuAssertDblEquals(tc, elements[0], *((TYPE*) list_get(list, 0)), 0);
    CuAssertDblEquals(tc, elements[2], *((TYPE*) list_get_checked(list, 1)), 0);
    CuAssertDblEquals(tc, elements[3], *((TYPE*) list_get_checked(list, 2)), 0);
    CuAssertPtrEquals(tc, NULL, list_get_checked(list, 3));
    CuAssertIntEquals(tc, 3, list_get_size(list));

    // check whether the next element is added at the right position
    list_add(list, &elements[4]);
    CuAssertDblEquals(tc, elements[4], *((TYPE*) list_get_checked(list, 3)), 0);
    CuAssertIntEquals(tc, 4, list_get_size(list));
}

void test_list_find(CuTest* tc) {
    List* list = new_list();
    list_add(list, &elements[0]);
    list_add(list, &elements[1]);
    list_add(list, &elements[2]);
    list_add(list, &elements[3]);

    CuAssertIntEquals(tc, 0, list_find_index(list, &elements[0]));
    CuAssertIntEquals(tc, 2, list_find_index(list, &elements[2]));
    CuAssertIntEquals(tc, 3, list_find_index(list, &elements[3]));

    ErrorCode ec = list_set(list, 2, &elements[4]);
    CuAssertIntEquals(tc, ERROR_NONE, ec);
    CuAssertIntEquals(tc, 2, list_find_index(list, &elements[4]));
}

void test_list_delete_value(CuTest* tc) {
    List* list = new_list();
    list_add(list, &elements[0]);
    list_add(list, &elements[1]);
    list_add(list, &elements[2]);
    list_add(list, &elements[3]);

    ErrorCode ec = list_delete_value(list, &elements[1]);

    CuAssertIntEquals(tc, ERROR_NONE, ec);
    CuAssertDblEquals(tc, elements[0], *((TYPE*) list_get_checked(list, 0)), 0);
    CuAssertDblEquals(tc, elements[2], *((TYPE*) list_get_checked(list, 1)), 0);
    CuAssertDblEquals(tc, elements[3], *((TYPE*) list_get_checked(list, 2)), 0);
    CuAssertPtrEquals(tc, NULL, list_get_checked(list, 3));
    CuAssertIntEquals(tc, 3, list_get_size(list));
}

void test_list_pop_push(CuTest* tc) {
    List* list = new_list();
    list_push(list, &elements[0]);
    list_push(list, &elements[1]);
    CuAssertIntEquals(tc, 2, list_get_size(list));

    list_push(list, &elements[2]);
    list_push(list, &elements[3]);
    CuAssertIntEquals(tc, 4, list_get_size(list));

    CuAssertDblEquals(tc, elements[3], *((TYPE*) list_pop(list)), 0);
    CuAssertDblEquals(tc, elements[2], *((TYPE*) list_pop(list)), 0);
    CuAssertIntEquals(tc, 2, list_get_size(list));

    CuAssertPtrEquals(tc, NULL, list_get_checked(list, 2));
    CuAssertDblEquals(tc, elements[1], *((TYPE*) list_get_checked(list, 1)), 0);
}

void test_list_iterators(CuTest* tc) {
    List* list = new_list();
    list_add(list, &elements[0]);
    list_add(list, &elements[1]);
    list_add(list, &elements[2]);
    list_add(list, &elements[3]);

    {
        ListIterator itr = list_iterator(list);
        int i = 0;

        while (list_iterator_has_next(&itr)) {
            const TYPE* ptr = list_iterator_next(&itr);

            CuAssertTrue(tc, i < 4); // i = [0 ... 3]
            CuAssertDblEquals(tc, elements[i], *ptr, 0);
            i++;
        }
        CuAssertIntEquals(tc, 4, i);
    }

    // check data corruption
    CuAssertDblEquals(tc, elements[0], *((TYPE*) list_get_checked(list, 0)), 0);
    CuAssertDblEquals(tc, elements[1], *((TYPE*) list_get_checked(list, 1)), 0);
    CuAssertDblEquals(tc, elements[2], *((TYPE*) list_get_checked(list, 2)), 0);
    CuAssertDblEquals(tc, elements[3], *((TYPE*) list_get_checked(list, 3)), 0);
    CuAssertPtrEquals(tc, NULL, list_get_checked(list, 4));
    CuAssertIntEquals(tc, 4, list_get_size(list));
}

void test_list_pack(CuTest* tc) {
    List* list = new_list();
    list_add(list, &elements[0]);
    list_add(list, &elements[1]);
    list_add(list, &elements[2]);
    list_add(list, &elements[3]);

    list_add(list, &elements[4]);
    list_add(list, &elements[5]);
    list_add(list, &elements[6]);
    list_add(list, &elements[7]);

    list_pop(list);
    list_pop(list);
    list_pop(list);
    list_pop(list);

    CuAssertIntEquals(tc, 4, list_get_size(list));

    size_t data_size_before_pack = list_get_data_size(list);

    list_pack(list);
    size_t data_size_after_pack = list_get_data_size(list);
    CuAssertTrue(tc, data_size_after_pack < data_size_before_pack);

    if (6 > list->_capacity) {
        _list_grow(list);
    }
    CuAssertTrue(tc, list_get_data_size(list) > data_size_after_pack);
}

void test_list_zero(CuTest* tc) {
    List* list = list_init(malloc(sizeof(List)), sizeof(TYPE), 0);
    // test for segfaults
    list_free(list);
    free(list);

    list = list_init(malloc(sizeof(List)), sizeof(TYPE), 0);
    list_add(list, &elements[0]);
    list_add(list, &elements[1]);

    CuAssertDblEquals(tc, elements[0], *((TYPE*) list_get_checked(list, 0)), 0);
    CuAssertDblEquals(tc, elements[1], *((TYPE*) list_get_checked(list, 1)), 0);

    list_free(list);
    free(list);

    list = list_init(malloc(sizeof(List)), sizeof(TYPE), 0);
    CuAssertPtrEquals(tc, NULL, list_get_checked(list, 0));
    CuAssertPtrEquals(tc, NULL, list_get_checked(list, 1));

    CuAssertIntEquals(tc, 0, list_get_size(list));

    CuAssertIntEquals(tc, ERROR_OUT_OF_BOUNDS, list_set(list, 0, &elements[0]));
    CuAssertIntEquals(tc, ERROR_OUT_OF_BOUNDS, list_set(list, 1, &elements[1]));

    CuAssertIntEquals(tc, ERROR_OUT_OF_BOUNDS, list_delete_index(list, 0));
    CuAssertPtrEquals(tc, NULL, list_pop(list));

    ListIterator iterator = list_iterator(list);
    CuAssertTrue(tc, !list_iterator_has_next(&iterator));

    list_clear(list);
    list_pack(list);
    CuAssertIntEquals(tc, 0, list_get_size(list));

    list_free(list);
    free(list);
}

void test_list_overload(CuTest* tc) {
    List* list = list_init(malloc(sizeof(List)), sizeof(TYPE), 8);

    list_add(list, &elements[0]);
    list_add(list, &elements[1]);
    list_add(list, &elements[2]);
    list_add(list, &elements[3]);
    list_add(list, &elements[4]);
    list_add(list, &elements[5]);
    list_add(list, &elements[6]);
    list_add(list, &elements[7]);
    list_add(list, &elements[8]);
    list_add(list, &elements[9]);
    list_add(list, &elements[0]);
    list_add(list, &elements[1]);

    // get
    CuAssertDblEquals(tc, elements[5], *((TYPE*) list_get_checked(list, 5)), 0);
    CuAssertDblEquals(tc, elements[0], *((TYPE*) list_get_checked(list, 10)), 0);
    CuAssertDblEquals(tc, elements[1], *((TYPE*) list_get_checked(list, 11)), 0);
    CuAssertPtrEquals(tc, NULL, list_get_checked(list, 12));
    CuAssertIntEquals(tc, 12, list_get_size(list));
    { // iterator
        ListIterator itr = list_iterator(list);
        int i = 0;

        while (list_iterator_has_next(&itr)) {
            const TYPE* ptr = list_iterator_next(&itr);

            CuAssertTrue(tc, i < 12);
            CuAssertDblEquals(tc, elements[i % 10], *ptr, 0);
            i++;
        }
        CuAssertIntEquals(tc, 12, i);
    }

    // find
    CuAssertIntEquals(tc, 3, list_find_index(list, &elements[3]));
    CuAssertIntEquals(tc, 9, list_find_index(list, &elements[9]));

    // delete
    ErrorCode ec = list_delete_index(list, 9);

    CuAssertIntEquals(tc, ERROR_NONE, ec);
    CuAssertDblEquals(tc, elements[0], *((TYPE*) list_get(list, 0)), 0);
    CuAssertDblEquals(tc, elements[8], *((TYPE*) list_get_checked(list, 8)), 0);
    CuAssertDblEquals(tc, elements[1], *((TYPE*) list_get_checked(list, 10)), 0);
    CuAssertPtrEquals(tc, NULL, list_get_checked(list, 11));
    CuAssertIntEquals(tc, 11, list_get_size(list));

    list_free(list);
    free(list);
}

CuSuite* list_suite(void) {
    CuSuite* suite = CuSuiteNew();
    alloc_list = allocator_sm_new(sizeof(List));

    SUITE_ADD_TEST(suite, test_list_add_get);
    SUITE_ADD_TEST(suite, test_list_set);
    SUITE_ADD_TEST(suite, test_list_get_checked);
    SUITE_ADD_TEST(suite, test_list_delete_index);
    SUITE_ADD_TEST(suite, test_list_delete_value);
    SUITE_ADD_TEST(suite, test_list_pop_push);
    SUITE_ADD_TEST(suite, test_list_find);
    SUITE_ADD_TEST(suite, test_list_iterators);
    SUITE_ADD_TEST(suite, test_list_pack);
    SUITE_ADD_TEST(suite, test_list_overload);

    return suite;
}

void list_cleanup() {
    allocator_sm_free(alloc_list);
}

#endif //YADF_TESTLIST_H
