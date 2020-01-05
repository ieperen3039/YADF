//
// A dynamic list that copies elements upon addition, and returns pointers to the entries upon retrieving.
// The list has no information hiding to encourage inlining
// Created by ieperen3039 on 25-11-19.
//

#ifndef YADF_LIST_C
#define YADF_LIST_C

#include <stdlib.h>
#include <string.h> // for memory operations
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include "global.h"

#define LIST_GROWTH_FACTOR_SMALL 2
#define LIST_GROWTH_SMALL_LIMIT 64
#define LIST_GROWTH_FACTOR_LARGE 1.2
#define LIST_MINIMUM_SIZE 4

typedef struct { // in-place definition to allow stack-allocation
    void* _data; // putting data first for fastest access
    size_t _element_size; // in chars (sizeof(Type))
    unsigned int _size; // in number of elements
    unsigned int _capacity; // in number of elements
} List;

/// see list_iterator_next and list_iterator_has_next
typedef struct { // in-place definition to allow stack-allocation
    size_t _element_size;
    void* _current;
    void* _end;
#ifndef NDEBUG
    const List* _src;
#endif
} ListIterator;

// an empty list
static const List LIST_EMPTY = {};

/**
 * initializes the given List.
 * element_size must be the sizeof(Type) for the type you plan on adding to this list.
 * Data must be added with calls to list_add(List*, void*) or list_set(List*, void*)
*/
static inline List* list_init(List* list, size_t element_size, int capacity) {
    assert(list != NULL);
    list->_size = 0;
    list->_capacity = capacity;
    list->_element_size = element_size;

    if (capacity > 0) {
        list->_data = malloc(element_size * capacity);
    } else {
        list->_data = NULL;
    }
    return list;
}

/// inverts an init operation, freeing allocated data (but not the list itself)
static void list_free(List* list) {
    free(list->_data);
}

/// get the address of the element at the given index without bounds checking
PURE static inline void* list_get(const List* list, int index) {
    return list->_data + (index * list->_element_size);
}

/// get a pointer to the element at the given index with bounds checking
PURE static inline void* list_get_checked(const List* list, int index) {
    if (index < 0 || index >= list->_size) {
        return NULL;
    }

    return list_get(list, index);
}

/// copy the element pointed to by *value* into this list at the given index
static inline ErrorCode list_set(List* list, int index, const void* value) {
    void* addr = list_get_checked(list, index);
    if (!addr) return ERROR_OUT_OF_BOUNDS;

    memcpy(addr, value, list->_element_size);

    return ERROR_NONE;
}

/// DO NOT USE
static void _list_grow_capacity(List* list, int minimum_size) {
    if (list->_capacity < LIST_MINIMUM_SIZE) {
        list->_capacity = LIST_MINIMUM_SIZE;

    } else {
        size_t data_size = list->_capacity * list->_element_size;
        float growth_factor = (data_size < LIST_GROWTH_SMALL_LIMIT ? LIST_GROWTH_FACTOR_SMALL
                                                                   : LIST_GROWTH_FACTOR_LARGE);
        list->_capacity = (int) ceilf(list->_capacity * growth_factor);
    }

    if (list->_capacity < minimum_size)
        list->_capacity = minimum_size;

    if (list->_data == NULL) {
        list->_data = malloc(list->_element_size * list->_capacity);

    } else {
        list->_data = realloc(list->_data, list->_element_size * list->_capacity);
    }
}

/**
 * Adds enough elements to the list to ensure the given size, or shrinks the list by deleting elements at the end.
 * Added elements have undefined values, and must be set using list_set
 * @param list
 * @param size
 */
static inline void list_set_size(List* list, int size) {
    if (size > list->_capacity) {
        _list_grow_capacity(list, size);
    }
    list->_size = size;
}

/// copy the element pointed to by *value* to the end of this list. The value must have the size used in the constructor of this object
static inline void* list_add(List* list, const void* value) {
    assert(value != NULL);
    if (list->_size + 1 > list->_capacity) {
        _list_grow_capacity(list, 0);
    }
    void* addr = list_get(list, list->_size++);
    memcpy(addr, value, list->_element_size);
    return addr;
}

/// removes a specific element from the list, and shifts all other elements one down.
/// See list_pop(List*) to remove and retrieve the last element.
static inline ErrorCode list_delete_index(List* list, int index) {
    void* tgt = list_get_checked(list, index);
    if (!tgt) return ERROR_OUT_OF_BOUNDS;

    size_t copy_size = ((list->_size - index) * list->_element_size);
    memmove(tgt, tgt + list->_element_size, copy_size);
    list->_size -= 1;

    return ERROR_NONE;
}

/// copy the element pointed to by value to the end of this list. (Stack operation)
static inline void list_push(List* list, const void* value) {
    list_add(list, value);
}

/// removes the element at the end of this list. (Stack operation) Returns NULL if list_empty(list)
static inline void* list_pop(List* list) {
    if (list->_size == 0) return NULL;
    return list_get(list, --list->_size);
}

/// returns the address of the given value in the list, or null if the value is not found
PURE static inline void* _list_value_address(const List* list, const void* value) {
    void* last = list_get(list, list->_size); // guard

    for (void* elt = list->_data; elt < last; elt += list->_element_size) {
        int diff = memcmp(elt, value, list->_element_size);
        if (diff == 0) return elt;
    }

    return NULL;
}

/// returns the index of the given value in the list, or -1 if no such value exists
PURE static inline int list_find_index(const List* list, const void* value) {
    void* ptr = _list_value_address(list, value);
    if (!ptr) return -1;
    return (int) ((ptr - list->_data) / list->_element_size);
}

/// removes a specific element from the list, and shifts all other elements one down. See list_pop(List*) to remove and retrieve the last element.
static inline ErrorCode list_delete_value(List* list, const void* value) {
    void* tgt = _list_value_address(list, value);
    if (!tgt) return ERROR_OUT_OF_BOUNDS;
    size_t copy_size = list_get(list, list->_size) - (tgt + list->_element_size);
    memmove(tgt, tgt + list->_element_size, copy_size);
    list->_size -= 1;

    return ERROR_NONE;
}

/**
 * Appends nr_of_values elements to this list
 * @param list the list to append to
 * @param values an array of values
 * @param nr_of_values the number of values in the array to be copied
 */
static inline void list_add_all(List* list, const void* values, int nr_of_values) {
    unsigned int new_size = list->_size + nr_of_values;
    if (new_size > list->_capacity) {
        _list_grow_capacity(list, new_size);
    }
    memcpy(list_get(list, list->_size), values, nr_of_values * list->_element_size);
    list->_size = new_size;
}

/// reallocates the current list to free memory resulting from deleted entries
static inline void list_pack(List* list) {
    list->_capacity = list->_size;

    if (list->_capacity > 0) {
        list->_data = realloc(list->_data, list->_element_size * list->_capacity);

    } else if (list->_data != NULL) {
        free(list->_data);
        list->_data = NULL;
    }
}

/// current number of elements in the list
PURE static inline int list_get_size(const List* list) {
    return (int) list->_size;
}

//. size of the data part of the list. The total size is (sizeof(List) + list_get_data_size())
PURE static inline size_t list_get_data_size(const List* list) {
    return list->_capacity * list->_element_size;
}

/// returns true iff (list_get_size(list) == 0)
PURE static inline bool list_is_empty(const List* list) {
    return list->_size == 0;
}

/**
 * clears all entities in this list.
 * This is an O(1) operation that only marks the contents invalid.
 * To reduce the memory footprint, one should also call list_pack
 */
static inline void list_clear(List* list) {
    list->_size = 0;
}

/**
 * swaps the contents of a and b, without too much data movement.
 * @param a one list
 * @param b another list
 */
static inline void list_swap_contents(List* a, List* b) {
    List temp;
    temp = *a;
    *a = *b;
    *b = temp;
}

/**
 * creates a deep copy of src, and writes it to dest.
 * The resulting list is minimally-sized, as if list_pack has been called on it
 * @param src the list to copy
 * @param dest the destination to copy to
 */
static inline void list_copy(const List* src, List* dest) {
    *dest = *src; // shallow: dest._data still points to src._data
    if (src->_data == NULL) return;

    dest->_data = malloc(src->_element_size * src->_size);
    memcpy(dest->_data, src->_data, src->_element_size * src->_size);
}

/**
 * Creates an iterator object for this list. Objects can be iterated through by list_iterator_next(ListIterator).
 * Any change to the size of this list invalidates this iterator, and using list_iterator_next after changes results in undefined behaviour.
 * This is likely to be faster than iterating over the index.
 */
PURE static inline ListIterator list_iterator(const List* list) {
    return (ListIterator) {
            list->_element_size,
            list->_data,
            list_get(list, list->_size),
    #ifndef NDEBUG
            list,
    #endif
    };
}

PURE static inline ListIterator list_iterator_empty() {
    return (ListIterator) {
            0,
            NULL,
            NULL,
    #ifndef NDEBUG
            NULL,
    #endif
    };
}

/// returns true iff there is an element left in the iterator, such that list_iterator_next will return a valid result.
PURE static inline bool list_iterator_has_next(ListIterator* iterator) {
    return (iterator->_current != iterator->_end);
};

/// returns a pointer to the next element from the given iterator. If !list_iterator_has_next(ListIterator), then the result is undefined (likely a segfault)
static inline void* list_iterator_next(ListIterator* iterator) {
#ifndef NDEBUG
    const List* list = iterator->_src;
    // check for concurrent modification
    assert(iterator->_end == list_get(list, list->_size));
#endif
    void* elt = iterator->_current;
    iterator->_current += iterator->_element_size;
    return elt;
}

/**
 * Creates an iterator object for the elements starting at index start_index (inclusive) until end_index (exclusive).
 * Objects can be iterated through by list_iterator_next(ListIterator).
 * Any change to the size of this list invalidates this iterator, and using list_iterator_next after changes results in undefined behaviour.
 * This is likely to be faster than iterating over the index.
 */
PURE static inline ListIterator list_sublist_iterator(const List* list, int start_index, int end_index) {
    return (ListIterator) {
            list->_element_size,
            list_get(list, start_index),
            list_get(list, end_index),
    #ifndef NDEBUG
            list,
    #endif
    };
}

#endif // YADF_LIST_C