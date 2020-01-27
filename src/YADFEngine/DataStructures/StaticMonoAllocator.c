//
// Created by ieperen3039 on 04-12-19.
//
#include "StaticMonoAllocator.h"

#ifndef ALLOCATOR_BLOCK_SIZE
    #define ALLOCATOR_BLOCK_SIZE 64
#endif // ALLOCATOR_BLOCK_SIZE

typedef void Type; // we don't know the actual size of the Type we allocate at compile time

struct _Static_Monotype_Alloctor {
    Type* next_elt;
    size_t element_size;
    int elements_left;
    List all_blocks;
};

AllocatorSM* allocator_sm_new(size_t element_size) {
    AllocatorSM* alloc = malloc(sizeof(AllocatorSM));
    list_init(&alloc->all_blocks, sizeof(Type*), 4);
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
    for (int i = 0; i < list_size(data_blocks); ++i) {
        Type* data_block = *((Type**) list_get(data_blocks, i));
        free(data_block);
    }
    list_free(data_blocks);
    free(alloc);
}

static void* allocator_sm(void* sm_alloc, size_t size) {
    if (size != ((AllocatorSM*) sm_alloc)->element_size){
        return NULL;
    }
    return allocator_sm_alloc(sm_alloc);
}

Allocator allocator_sm_get(AllocatorSM* elt) {
    Allocator result = {elt, allocator_sm,};
    return result;
}

