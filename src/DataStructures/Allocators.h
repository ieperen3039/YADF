//
// Created by s152717 on 14-12-2019.
//

#ifndef YADF_ALLOCATORS_H
#define YADF_ALLOCATORS_H

typedef struct _Allocator {
    void* object;

    void* (* get)(void* this, size_t);

    void (* free)(void* element);
} Allocator;

static inline void* alloc_get(Allocator a, size_t size) {
    return a.get(a.object, size);
}

static inline void alloc_free(Allocator a, void* element) {
    a.free(element);
}

/// malloc(size_t) as an Allocator
static inline void* _system_malloc(void* null, size_t size) {
    return malloc(size);
}

#define SYSTEM_ALLOCATOR (Allocator){NULL, _system_malloc, free}

#endif //YADF_ALLOCATORS_H
