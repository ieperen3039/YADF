//
// Created by ieperen3039 on 04-12-19.
//
#include "StaticMonoAllocator.h"

typedef void Type; // we don't know the actual size of the Type we allocate at compile time

struct _Static_Monotype_Alloctor {
    Type* next_elt;
    size_t element_size;
    int elements_left;
    List all_blocks;
};

AllocatorSM* allocator_sm_new(size_t element_size) {
    AllocatorSM* alloc = malloc(sizeof(AllocatorSM));
    int initial_elements = 4;
    // abuse the fact that List is a complete type
    alloc->all_blocks = (List) {
            ._data = malloc(sizeof(Type*) * initial_elements),
            ._size = 0, ._capacity = initial_elements, ._element_size = sizeof(Type*)
    };
    alloc->elements_left = 0;
    alloc->element_size = element_size;
    return alloc;
}

Type* allocator_sm_alloc(AllocatorSM* alloc) {
    if (alloc->elements_left == 0) {
        alloc->next_elt = malloc(ALLOCATOR_BLOCK_SIZE * alloc->element_size);
        alloc->elements_left = ALLOCATOR_BLOCK_SIZE;
        list_add(&alloc->all_blocks, &alloc->next_elt);
    }

    Type* next = alloc->next_elt;
    alloc->next_elt += alloc->element_size;
    alloc->elements_left--;
    return next;
}

void allocator_sm_free(AllocatorSM* alloc) {
    List* data_blocks = &alloc->all_blocks;
    for (int i = 0; i < list_get_size(data_blocks); ++i) {
        Type* data_block = *((Type**) list_get_unsafe(data_blocks, i));
        free(data_block);
    }
    list_free(data_blocks);
    free(alloc);
}

