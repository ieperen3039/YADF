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

#define WAIT_FOR_THREADS

#define BUFFER_SIZE (WORKER_COUNT + 1)
#define NR_OF_UPDATE_LISTS (WORKER_COUNT * 2)
#define LISTS_REALLOC_INTERVAL 360000


struct _UpdateWorkerPool {
    struct {
        unsigned long thread_id;
    } workers[WORKER_COUNT];

    HANDLE loop_limiter;
    unsigned long dispatch_thread;
    World* world;

    CRITICAL_SECTION lock_buffer;
    CONDITION_VARIABLE cond_has_space;
    CONDITION_VARIABLE cond_has_elements;
    CONDITION_VARIABLE cond_idle_workers;

    volatile enum State state;
    volatile int idle_workers;

    volatile int buffer_head;
    volatile int buffer_tail;

    volatile UpdateCycle game_time;
    volatile ListIterator buffer[BUFFER_SIZE];

    List update_entities[NR_OF_UPDATE_LISTS]; // only accessed by the dispatcher
};

void worker_loop(void* worker_pool_ptr) {
    UpdateWorkerPool* pool = worker_pool_ptr;
    ListIterator entities;
    UpdateCycle game_time;

    while (pool->state != STATE_STOPPING) {
        EnterCriticalSection(&pool->lock_buffer);

        while (pool->buffer_head == pool->buffer_tail) {
            pool->idle_workers++;
            WakeConditionVariable(&pool->cond_idle_workers);

            SleepConditionVariableCS(&pool->cond_has_elements, &pool->lock_buffer, INFINITE);
            if (pool->state == STATE_STOPPING) return;

            pool->idle_workers--;
        }

        game_time = pool->game_time;
        enum State state = pool->state;
        int tail = pool->buffer_tail;
        entities = pool->buffer[tail];
        pool->buffer_tail = (tail + 1) % BUFFER_SIZE;

        WakeConditionVariable(&pool->cond_has_space);
        LeaveCriticalSection(&pool->lock_buffer);

        update(&entities, game_time, state);
    }

    // optional cleanup
}

void sync_workers(UpdateWorkerPool* pool, enum State new_state) {
    EnterCriticalSection(&pool->lock_buffer);
    while (pool->idle_workers < WORKER_COUNT) {
        SleepConditionVariableCS(&pool->cond_idle_workers, &pool->lock_buffer, INFINITE);
    }

    pool->state = new_state;
    LeaveCriticalSection(&pool->lock_buffer);
}

void dispatch(UpdateWorkerPool* pool, ListIterator batch) {
    EnterCriticalSection(&pool->lock_buffer);

    while (((pool->buffer_head + 1) % BUFFER_SIZE) == pool->buffer_tail) {
        SleepConditionVariableCS(&pool->cond_has_space, &pool->lock_buffer, INFINITE);
    }

    // set buffer
    int head = pool->buffer_head;
    pool->buffer[head] = batch;
    pool->buffer_head = (head + 1) % BUFFER_SIZE;

    LeaveCriticalSection(&pool->lock_buffer);
    WakeConditionVariable(&pool->cond_has_elements);
}

void update_dispatch(void* data) {
    UpdateWorkerPool* pool = data;
    List entities;

    while (pool->state != STATE_STOPPING) {
        pool->game_time++;
        update_move_entities(pool->world, &entities);

        // we assume to be already synced with workers
        assert(pool->idle_workers == WORKER_COUNT);
        const int nr_of_entities = list_get_size(&entities);
        const int elements_per_worker = max_i(nr_of_entities / WORKER_COUNT + 1, 16);

        // TODO Lock callbacks

        int next_batch_start = 0;
        while (next_batch_start < nr_of_entities) {
            int end_index = min_i(next_batch_start + elements_per_worker, nr_of_entities);

            ListIterator batch = list_sublist_iterator(&entities, next_batch_start, end_index);
            dispatch(pool, batch);

            next_batch_start = end_index;
        }

        for (int i = 0; i < NR_OF_UPDATE_LISTS; ++i) {
            list_clear(&pool->update_entities[i]);
        }

        // sync with workers
        sync_workers(pool, STATE_POST_UPDATE);

        ListIterator entities_itr = list_iterator(&entities);
        update_sort_entities_on_position(&entities_itr, pool->update_entities, NR_OF_UPDATE_LISTS);

        // TODO Lock frame rendering

        for (int i = 0; i < NR_OF_UPDATE_LISTS; ++i) {
            List* list_to_update = &pool->update_entities[i];

            if (!list_is_empty(list_to_update)) {
                ListIterator batch = list_iterator(list_to_update);
                dispatch(pool, batch);
            }
        }

        // await workers to unlock frame rendering
        sync_workers(pool, STATE_PRE_UPDATE);

        // TODO Unlock frame rendering

        WaitForSingleObject(&pool->loop_limiter, INFINITE);
    }
}

UpdateWorkerPool* update_workers_new() {
    UpdateWorkerPool* pool = malloc(sizeof(UpdateWorkerPool));
    pool->state = STATE_PRE_UPDATE;
    pool->buffer_head = 0;
    pool->buffer_tail = 0;
    pool->idle_workers = 0;

    pool->loop_limiter = CreateSemaphore(NULL, 1, 1, NULL);
    InitializeCriticalSection(&pool->lock_buffer);
    InitializeConditionVariable(&pool->cond_has_space);
    InitializeConditionVariable(&pool->cond_has_elements);
    InitializeConditionVariable(&pool->cond_idle_workers);


    for (int i = 0; i < WORKER_COUNT; ++i) {
        unsigned long thread_handle = _beginthread(worker_loop, 1, pool);
        assert(thread_handle >= 0);
        pool->workers[i].thread_id = thread_handle;
    }

    sync_workers(pool, STATE_PRE_UPDATE);

    return pool;
}

void update_workers_free(UpdateWorkerPool* pool) {
    EnterCriticalSection(&pool->lock_buffer);
    pool->state = STATE_STOPPING;
    LeaveCriticalSection(&pool->lock_buffer);

    WakeAllConditionVariable(&pool->cond_has_elements);

#ifdef WAIT_FOR_THREADS
    HANDLE handle[WORKER_COUNT];
    for (int i = 0; i < WORKER_COUNT; ++i) {
        handle[i] = &pool->workers[i].thread_id;
    }

    WaitForMultipleObjects(WORKER_COUNT, handle, true, 2000);
#endif

    free(pool);
}

void update_start_tick(UpdateWorkerPool* pool, World* world) {
    pool->world = world;

    WaitForSingleObject(pool->loop_limiter, 0);
    ReleaseSemaphore(pool->loop_limiter, 1, NULL);
}
