//
// Created by s152717 on 29-12-2019.
//

#if _WIN32_WINNT < 0x0501
    #undef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
#endif

#include "UpdateEngine.h"
#include "UpdateEngineShared.h"
#include "../Entities/EntityInstance.h"
#include "../World/World.h"

#include <process.h>
#include <windows.h>

#ifndef WORKER_COUNT
#define WORKER_COUNT 2
#endif

#define WAIT_FOR_THREADS true

struct _UpdateWorkerPool {
    struct {
        unsigned long thread_id;
    } workers[WORKER_COUNT];

    CRITICAL_SECTION condition_lock;
    CONDITION_VARIABLE cond_has_elements;

    ListIterator buffer;
    volatile bool has_data;
    volatile bool keep_going;
};

void worker_loop(void* worker_pool_ptr) {
    UpdateWorkerPool* pool = worker_pool_ptr;
    ListIterator tasks;

    while (pool->keep_going) {

        EnterCriticalSection(&pool->condition_lock);
        while (!pool->has_data && pool->keep_going) {
            // Buffer is full - sleep so consumers can get items.
            SleepConditionVariableCS(&pool->cond_has_elements, &pool->condition_lock, INFINITE);
        }

        if (!pool->keep_going) {
            LeaveCriticalSection(&pool->condition_lock);
            break;
        }

        tasks = pool->buffer;
        pool->buffer = list_iterator_empty();
        pool->has_data = false;

        LeaveCriticalSection(&pool->condition_lock);

        update_entities_pre(tasks, 0);
    }

    // optional cleanup
}

UpdateWorkerPool* update_workers_new() {
    UpdateWorkerPool* pool = malloc(sizeof(UpdateWorkerPool));
    pool->keep_going = 1;
    InitializeCriticalSection(&pool->condition_lock);
    InitializeConditionVariable(&pool->cond_has_elements);

    for (int i = 0; i < WORKER_COUNT; ++i) {
        unsigned long thread_handle = _beginthread(worker_loop, 1, pool);
        assert(thread_handle >= 0);
        pool->workers[i].thread_id = thread_handle;
    }

    return pool;
}

void update_workers_free(UpdateWorkerPool* pool) {
    EnterCriticalSection(&pool->condition_lock);
    pool->keep_going = 0;
    LeaveCriticalSection(&pool->condition_lock);

    WakeAllConditionVariable(&pool->cond_has_elements);

    if (WAIT_FOR_THREADS) {
        HANDLE handle[WORKER_COUNT];
        for (int i = 0; i < WORKER_COUNT; ++i) {
            handle[i] = &pool->workers[i].thread_id;
        }

        WaitForMultipleObjects(WORKER_COUNT, handle, true, 2000);
    }

    free(pool);
}

void update_world_tick(World* world, UpdateCycle game_time, UpdateWorkerPool* worker_pool) {
    assert(world != NULL && worker_pool != NULL);

    List* src_list = world_get_entities_to_update(world);
    List list;
    list_init(&list, src_list->_element_size, WORLD_DEFAULT_ENTITY_LIST_CAPACITY);
    list_swap_contents(&list, src_list);

    int nr_of_entities = list_get_size(&list);
    int elements_per_worker = max_i(nr_of_entities / WORKER_COUNT + 1, 16);
    int next_batch_start = 0;

    while (next_batch_start < nr_of_entities) {
        int end_index = min_i(next_batch_start + elements_per_worker, nr_of_entities);
        ListIterator batch = list_sublist_iterator(&list, next_batch_start, end_index);

        EnterCriticalSection(&worker_pool->condition_lock);
        // set buffer
        worker_pool->buffer = batch;
        worker_pool->has_data = true;

        LeaveCriticalSection(&worker_pool->condition_lock);
        WakeConditionVariable(&worker_pool->cond_has_elements);

        next_batch_start = end_index;
    }
}
