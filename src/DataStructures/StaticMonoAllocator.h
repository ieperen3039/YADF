//
// Created by ieperen3039 on 04-12-19.
//

#ifndef YADF_STATICMONOALLOCATOR_H
#define YADF_STATICMONOALLOCATOR_H

#include <stdlib.h>

#include "List.h"
#ifndef ALLOCATOR_BLOCK_SIZE
#define ALLOCATOR_BLOCK_SIZE 64
#endif // ALLOCATOR_BLOCK_SIZE

typedef struct _Static_Monotype_Alloctor AllocatorSM;

/**
 * creates a static initialisation, mono-type allocator with initial space of one block (ALLOCATOR_BLOCK_SIZE elements)
 * static : allocated elements can't be freed until the end
 * mono-type : only one type (size) can be allocated
 */
AllocatorSM* allocator_sm_new(size_t element_size);

/// allocates a new element. The allocated element may not be freed _at all_ except when free-ing this allocator using allocator_sm_free
void* allocator_sm_alloc(AllocatorSM* alloc);

/// frees this allocator and all allocated elements.
/// all elements allocated by this allocator will be freed
void allocator_sm_free(AllocatorSM* alloc);

#endif //YADF_STATICMONOALLOCATOR_H
