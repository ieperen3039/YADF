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
#include "global.h"

#define GROWTH_FACTOR_SMALL 2
#define GROWTH_SMALL_LIMIT 64
#define GROWTH_FACTOR_LARGE 1.2

typedef struct { // in-place definition to allow stack-allocation
    void* _data;
    int _size; // in number of elements
    int _capacity; // in number of elements
    size_t _element_size; // in chars (sizeof(Type))
} List;

typedef struct { // in-place definition to allow stack-allocation
    size_t _element_size;
    void* _current;
    void* _end;
} ListIterator;

/**
 * Creates and initializes a new List.
 * data_size must be the sizeof(Type) for the type you plan on adding to this list.
 * Data must be added with calls to list_add(List*, void*) or list_set(List*, void*)
*/
static inline List* list_new(size_t data_size, int capacity) {
    List* ptr = malloc(sizeof(List));
    ptr->_data = malloc(data_size * capacity);
    ptr->_size = 0;
    ptr->_capacity = capacity;
    ptr->_element_size = data_size;
    return ptr;
}

/// frees this list and all data associated with it
static void list_free(List* vector) {
    free(vector->_data);
    free(vector);
}

/// get the address of the element at the given index without bounds checking
PURE static inline void* list_get_unsafe(const List* vector, int index) {
    return vector->_data + (index * vector->_element_size);
}

/// get a pointer to the element at the given index with bounds checking
PURE static inline void* list_get(List* vector, int index) {
    if (index < 0 || index >= vector->_size) {
        return NULL;
    }

    return list_get_unsafe(vector, index);
}

/// copy the element pointed to by *value* into this list at the given index
static inline ErrorCode list_set(List* vector, int index, void* value) {
    void* addr = list_get(vector, index);
    if (!addr) return ERROR_NONE;

    memcpy(addr, value, vector->_element_size);

    return ERROR_OUT_OF_BOUNDS;
}

/// ensures that the vector can contain *new_size* elements
static inline void list_resize(List* vector, int new_size) {
    if (new_size >= vector->_capacity) {
        double growth_factor = (vector->_capacity < GROWTH_SMALL_LIMIT ? GROWTH_FACTOR_SMALL : GROWTH_FACTOR_LARGE);
        vector->_capacity = (int) (vector->_capacity * growth_factor + 1);
        vector->_data = realloc(vector->_data, vector->_element_size * vector->_capacity);
    }
}

/// copy the element pointed to by *value* to the end of this list. The value must have the size used in the constructor of this object
static inline void list_add(List* vector, void* value) {
    list_resize(vector, vector->_size);
    void* addr = list_get_unsafe(vector, vector->_size++);
    memcpy(addr, value, vector->_element_size);
}

/// removes a specific element from the list, and shifts all other elements one down.
/// See list_pop(List*) to remove and retrieve the last element.
static inline ErrorCode list_delete_index(List* vector, int index) {
    void* tgt = list_get(vector, index);
    if (!tgt) return ERROR_NONE;

    size_t copy_size = ((vector->_size - index) * vector->_capacity);
    memmove(tgt + vector->_element_size, tgt, copy_size);
    vector->_size -= 1;

    return ERROR_OUT_OF_BOUNDS;
}

/// copy the element pointed to by value to the end of this list
static inline void list_push(List* vector, void* value) {
    list_add(vector, value);
}

/// removes the element at the end of this list.
static inline void* list_pop(List* vector) {
    return list_get_unsafe(vector, --vector->_size);
}

/// returns the address of the given value in the list, or null if the value is not found
PURE static inline void* value_address(List* vector, void* value) {
    void* last = list_get_unsafe(vector, vector->_size);
    for (void* elt = vector->_data; elt < last; elt += vector->_element_size) {
        bool equal = memcmp(elt, value, vector->_element_size);
        if (equal) return elt;
    }

    return NULL;
}

/// returns the index of the given value in the list, or -1 if no such value exists
PURE static inline int list_find_index(List* vector, void* value) {
    void* ptr = value_address(vector, value);
    if (!ptr) return -1;
    return (int) ((ptr - vector->_data) / vector->_element_size);
}

/// removes a specific element from the list, and shifts all other elements one down. See list_pop(List*) to remove and retrieve the last element.
static inline ErrorCode list_delete_value(List* vector, void* value) {
    void* tgt = value_address(vector, value);
    if (!tgt) return ERROR_NONE;
    size_t copy_size = list_get_unsafe(vector, vector->_size) - (tgt + vector->_element_size);
    memmove(tgt + vector->_element_size, tgt, copy_size);
    vector->_size -= 1;

    return ERROR_OUT_OF_BOUNDS;
}

/// reallocates the current list to free memory resulting from deleted entries
static inline void list_pack(List* vector) {
    vector->_capacity = vector->_size;
    vector->_data = realloc(vector->_data, vector->_element_size * vector->_capacity);
}

/// current number of elements in the list
PURE static inline int list_get_size(List* vector) {
    return vector->_size;
}

/// returns true iff (list_get_size(vector) == 0)
PURE static inline bool list_is_empty(List* vector) {
    return vector->_size == 0;
}

/**
 * Creates an iterator object for this list. Objects can be iterated through by list_iterator_next(ListIterator)
 * Any change to this list invalidates this iterator, and using list_iterator_next after changes results in undefined behaviour.
 * This is likely to be faster than iterating over the index.
 */
PURE static inline ListIterator list_iterator(List* vector) {
    return (ListIterator) {
            vector->_element_size,
            vector->_data,
            vector->_data + (vector->_size * vector->_element_size)
    };
}

/// returns true iff there is an element left in the iterator, such that list_iterator_next will return a valid result.
PURE static inline bool list_iterator_has_next(ListIterator* iterator) {
    return (iterator->_current != iterator->_end);
};

/// gets the next element from the given iterator. If !list_iterator_has_next(ListIterator), then the result is undefined (likely a segfault)
static inline void* list_iterator_next(ListIterator* iterator) {
    void* elt = iterator->_current;
    iterator->_current += iterator->_element_size;
    return elt;
}

#endif // YADF_LIST_C