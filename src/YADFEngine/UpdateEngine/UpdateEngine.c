//
// Created by s152717 on 29-12-2019.
//

#include "UpdateEngine.h"
#include "UpdateEngineActions.h"
#include "Sync.h"

#ifndef WORKER_COUNT
#define WORKER_COUNT 2
#endif

#define BUFFER_SIZE (WORKER_COUNT + 1)
#define NR_OF_UPDATE_LISTS (WORKER_COUNT * 2)

struct _UpdateWorkerPool {
    sync_semaphore loop_limiter;
    World* world;

    sync_mutex lock_buffer;
    sync_condition cond_has_space;
    sync_condition cond_has_elements;
    sync_condition cond_idle_workers;

    volatile enum State state;
    volatile int idle_workers; // waiting or stopped

    volatile int buffer_head;
    volatile int buffer_tail;

    volatile UpdateCycle game_time;
    volatile ListIterator buffer[BUFFER_SIZE];

    List update_entities[NR_OF_UPDATE_LISTS]; // only accessed by the dispatcher
};

void* worker_function(void* worker_pool_ptr) {
    UpdateWorkerPool* pool = worker_pool_ptr;
    ListIterator task_data;
    UpdateCycle game_time;

    while (pool->state != STATE_STOPPING) {
        sync_lock(&pool->lock_buffer);

        while (pool->buffer_head == pool->buffer_tail) {
            pool->idle_workers++;
            sync_condition_signal(&pool->cond_idle_workers);

            sync_condition_wait(&pool->cond_has_elements, &pool->lock_buffer);

            if (pool->state == STATE_STOPPING) {
                // thread stops as idle, and should return all locks
                sync_unlock(&pool->lock_buffer);
                return NULL;
            }

            pool->idle_workers--;
        }

        game_time = pool->game_time;
        enum State state = pool->state;
        int tail = pool->buffer_tail;
        task_data = pool->buffer[tail];
        pool->buffer_tail = (tail + 1) % BUFFER_SIZE;

        sync_condition_signal(&pool->cond_has_space);
        sync_unlock(&pool->lock_buffer);

        update(&task_data, game_time, state);
    }

    // a stopped thread is an idle thread
    pool->idle_workers++;
    sync_condition_signal(&pool->cond_idle_workers);

    return NULL;
}

/**
 * Wait until all workers in the given pool are in idle state. This non-idle workers.
 * More specifically, it waits until @code pool->idle_workers @endcode is set to WORKER_COUNT
 * @param pool the worker pool to wait for
 * @param new_state the state upon exit, unless STATE_STOPPING has been set
 */
void synchronize_workers(UpdateWorkerPool* pool, enum State new_state) {
    sync_lock(&pool->lock_buffer);
    while (pool->idle_workers < WORKER_COUNT) {
        sync_condition_wait(&pool->cond_idle_workers, &pool->lock_buffer);
    }

    if (pool->state != STATE_STOPPING){
        pool->state = new_state;
    }

    sync_unlock(&pool->lock_buffer);
}

void dispatch_task(UpdateWorkerPool* pool, ListIterator task) {
    sync_lock(&pool->lock_buffer);

    while (((pool->buffer_head + 1) % BUFFER_SIZE) == pool->buffer_tail) {
        sync_condition_wait(&pool->cond_has_space, &pool->lock_buffer);
    }

    // set buffer
    int head = pool->buffer_head;
    pool->buffer[head] = task;
    pool->buffer_head = (head + 1) % BUFFER_SIZE;

    sync_unlock(&pool->lock_buffer);
    sync_condition_signal(&pool->cond_has_elements);
}

void* update_dispatch(void* data) {
    UpdateWorkerPool* pool = data;
    List entities;

    sync_semaphore_wait(&pool->loop_limiter);

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
            dispatch_task(pool, batch);

            next_batch_start = end_index;
        }

        for (int i = 0; i < NR_OF_UPDATE_LISTS; ++i) {
            list_clear(&pool->update_entities[i]);
        }

        // sync with workers
        synchronize_workers(pool, STATE_POST_UPDATE);

        ListIterator entities_itr = list_iterator(&entities);
        update_sort_entities_on_position(&entities_itr, pool->update_entities, NR_OF_UPDATE_LISTS);

        // TODO Lock frame rendering

        for (int i = 0; i < NR_OF_UPDATE_LISTS; ++i) {
            List* list_to_update = &pool->update_entities[i];

            if (!list_is_empty(list_to_update)) {
                ListIterator batch = list_iterator(list_to_update);
                dispatch_task(pool, batch);
            }
        }

        // await workers to unlock frame rendering
        synchronize_workers(pool, STATE_PRE_UPDATE);

        // TODO Unlock frame rendering

        sync_semaphore_wait(&pool->loop_limiter);
    }

    // this ensures that iff (idle_workers > WORKER_COUNT) then all threads are stopped
    pool->idle_workers++;
    sync_condition_signal(&pool->cond_idle_workers);

    return NULL;
}

UpdateWorkerPool* update_workers_new() {
    UpdateWorkerPool* pool = malloc(sizeof(UpdateWorkerPool));
    pool->state = STATE_PRE_UPDATE;
    pool->buffer_head = 0;
    pool->buffer_tail = 0;
    pool->idle_workers = 0;

    pool->loop_limiter = sync_semaphore_new(0, 1);
    pool->lock_buffer = sync_mutex_new();
    pool->cond_has_space = sync_condition_new();
    pool->cond_has_elements = sync_condition_new();
    pool->cond_idle_workers = sync_condition_new();

    for (int i = 0; i < NR_OF_UPDATE_LISTS; ++i) {
        list_init(&pool->update_entities[i], sizeof(struct EntityUpdateElt), 256 / NR_OF_UPDATE_LISTS);
    }

    for (int i = 0; i < WORKER_COUNT; ++i) {
        sync_new_thread(worker_function, pool);
    }

    synchronize_workers(pool, STATE_PRE_UPDATE);

    sync_new_thread(update_dispatch, pool);

    return pool;
}

void update_workers_free(UpdateWorkerPool* pool) {
    sync_lock(&pool->lock_buffer);
    pool->state = STATE_STOPPING;
    sync_unlock(&pool->lock_buffer);

    // wake all threads waiting for tasks
    sync_condition_broadcast(&pool->cond_has_elements);

    // wait until all threads are stopped
    // iff (idle_workers > WORKER_COUNT) then all threads are stopped
    sync_lock(&pool->lock_buffer);
    while (pool->idle_workers <= WORKER_COUNT) {
        sync_condition_wait(&pool->cond_idle_workers, &pool->lock_buffer);
    }
    sync_unlock(&pool->lock_buffer);

    free(pool);
}

void update_start_tick(UpdateWorkerPool* pool, World* world) {
    pool->world = world;

    sync_semaphore_trywait(&pool->loop_limiter);
    sync_semaphore_post(&pool->loop_limiter);
}
