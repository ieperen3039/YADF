//
// Created by s152717 on 13-12-2019.
//

#include "UpdateEngine.h"
#include "UpdateEngineActions.h"
#include <stdlib.h>
#include "Sync.h"

#ifndef WORKER_COUNT
#define WORKER_COUNT 7
#endif

#define BUFFER_SIZE (WORKER_COUNT + 1)
#define NR_OF_UPDATE_LISTS (WORKER_COUNT * 2)
#define LISTS_REALLOC_INTERVAL 360000

struct _UpdateWorkerPool {
    struct {
        pthread_t thread_id;
    } workers[WORKER_COUNT];

    sem_t loop_limiter;
    pthread_t dispatcher;
    volatile World* world;

    pthread_mutex_t lock_buffer;
    pthread_cond_t cond_has_space;
    pthread_cond_t cond_has_elements;
    pthread_cond_t cond_idle_workers;

    volatile enum State state;
    volatile int idle_workers;

    volatile int buffer_head;
    volatile int buffer_tail;

    volatile UpdateCycle game_time;
    volatile ListIterator buffer[BUFFER_SIZE];

    List update_entities[NR_OF_UPDATE_LISTS]; // only accessed by the dispatcher
};

/// callback for pthread_create
void* worker_loop(void* worker_pool_ptr) {
    UpdateWorkerPool* pool = worker_pool_ptr;
    ListIterator entities;
    UpdateCycle game_time;

    while (pool->state != STATE_STOPPING) {
        pthread_mutex_lock(&pool->lock_buffer);

        while (pool->buffer_head == pool->buffer_tail) {
            pool->idle_workers++;
            pthread_cond_signal(&pool->cond_idle_workers);

            pthread_cond_wait(&pool->cond_has_elements, &pool->lock_buffer);
            if (pool->state == STATE_STOPPING) return NULL;

            pool->idle_workers--;
        }

        game_time = pool->game_time;
        enum State state = pool->state;
        int tail = pool->buffer_tail;
        entities = pool->buffer[tail];
        pool->buffer_tail = (tail + 1) % BUFFER_SIZE;

        pthread_cond_signal(&pool->cond_has_space);
        pthread_mutex_unlock(&pool->lock_buffer);

        update(&entities, game_time, state);
    }

    return NULL;
}

void sync_workers(UpdateWorkerPool* pool, enum State new_state) {
    pthread_mutex_lock(&pool->lock_buffer);
    while (pool->idle_workers < WORKER_COUNT) {
        pthread_cond_wait(&pool->cond_idle_workers, &pool->lock_buffer);
    }

    pool->state = new_state;
    pthread_mutex_unlock(&pool->lock_buffer);
}

void dispatch(UpdateWorkerPool* pool, ListIterator batch) {
    pthread_mutex_lock(&pool->lock_buffer);
    // if necessary, wait until there is space in the buffer
    while (((pool->buffer_head + 1) % BUFFER_SIZE) == pool->buffer_tail) {
        pthread_cond_wait(&pool->cond_has_space, &pool->lock_buffer);
    }

    // set buffer
    int head = pool->buffer_head;
    pool->buffer[head] = batch;
    pool->buffer_head = (head + 1) % BUFFER_SIZE;

    pthread_mutex_unlock(&pool->lock_buffer);
    // signal buffer available
    pthread_cond_signal(&pool->cond_has_elements);
}

void* update_dispatch(void* data){
    UpdateWorkerPool* pool = data;
    List entities;

    sem_wait(&pool->loop_limiter);

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

            if (!list_is_empty(list_to_update)){
                ListIterator batch = list_iterator(list_to_update);
                dispatch(pool, batch);
            }
        }

        // await workers to unlock frame rendering
        sync_workers(pool, STATE_PRE_UPDATE);

        // TODO Unlock frame rendering

        sem_wait(&pool->loop_limiter);
    }

    return NULL;
}

UpdateWorkerPool* update_workers_new() {
    UpdateWorkerPool* pool = malloc(sizeof(UpdateWorkerPool));
    pool->state = STATE_PRE_UPDATE;
    pool->buffer_head = 0;
    pool->buffer_tail = 0;
    pool->idle_workers = 0;

    update_init_sort_lists(pool->update_entities, NR_OF_UPDATE_LISTS, 1000);

    pthread_mutex_init(&pool->lock_buffer, NULL);
    pthread_cond_init(&pool->cond_has_space, NULL);
    pthread_cond_init(&pool->cond_has_elements, NULL);
    pthread_cond_init(&pool->cond_idle_workers, NULL);

    for (int i = 0; i < WORKER_COUNT; ++i) {
        pthread_create(&pool->workers[i].thread_id, NULL, worker_loop, pool);
    }

    sync_workers(pool, STATE_PRE_UPDATE);
    pthread_create(&pool->dispatcher, NULL, update_dispatch, pool);

    return pool;
}

void update_workers_free(UpdateWorkerPool* pool) {
    pthread_mutex_lock(&pool->lock_buffer);
    pool->state = STATE_STOPPING;
    pthread_cond_broadcast(&pool->cond_has_elements);
    pthread_mutex_unlock(&pool->lock_buffer);

    for (int i = 0; i < WORKER_COUNT; ++i) {
        pthread_join(pool->workers[i].thread_id, NULL);
    }

    free(pool);
}

void update_start_tick(UpdateWorkerPool* pool, World* world) {
    pool->world = world;

    sem_trywait(&pool->loop_limiter); // discard standing posts
    sem_post(&pool->loop_limiter);
}