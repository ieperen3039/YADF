//
// Created by s152717 on 13-12-2019.
//

#include "UpdateEngine.h"
#include "UpdateEngineShared.h"
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#ifndef WORKER_COUNT
#define WORKER_COUNT 7
#endif
#define BUFFER_SIZE (WORKER_COUNT + 1)

enum State{
    STATE_STOPPING, STATE_PRE_UPDATE, STATE_POST_UPDATE
};

struct _UpdateWorkerPool {
    struct {
        pthread_t thread_id;
    } workers[WORKER_COUNT];

    volatile int buffer_head;
    volatile int buffer_tail;
    volatile enum State state;
    volatile ListIterator buffer[BUFFER_SIZE];
    volatile UpdateCycle game_time;

    pthread_mutex_t lock_has_space;
    pthread_cond_t cond_has_space;
    
    pthread_mutex_t lock_has_elements;
    pthread_cond_t cond_has_elements;
};

void* worker_loop(void* worker_pool_ptr) {
    UpdateWorkerPool* pool = worker_pool_ptr;
    ListIterator entities;
    UpdateCycle game_time;

    while (pool->state != STATE_STOPPING) {
        pthread_mutex_lock(&pool->lock_has_elements);

        while (pool->buffer_head == pool->buffer_tail) {
            pthread_cond_wait(&pool->cond_has_elements, &pool->lock_has_elements);
            pthread_cond_signal(&pool->cond_has_space);
        }

        int tail = pool->buffer_tail;
        entities = pool->buffer[tail];
        pool->buffer_tail = (tail + 1) % BUFFER_SIZE;
        game_time = pool->game_time;
        
        pthread_mutex_unlock(&pool->lock_has_elements);

        update_entities_pre(entities, game_time);
    }

    return NULL;
}

UpdateWorkerPool* update_workers_new() {
    UpdateWorkerPool* pool = malloc(sizeof(UpdateWorkerPool));
    pool->state = STATE_PRE_UPDATE;
    pool->buffer_head = 0;
    pool->buffer_tail = 0;
    pthread_mutex_init(&pool->lock_has_elements, NULL);
    pthread_cond_init(&pool->cond_has_elements, NULL);

    for (int i = 0; i < WORKER_COUNT; ++i) {
        pthread_create(&pool->workers[i].thread_id, NULL, worker_loop, pool);
    }

    return pool;
}

void update_workers_free(UpdateWorkerPool* pool) {
    pthread_mutex_lock(&pool->lock_has_elements);
    pool->state = STATE_STOPPING;
    pthread_cond_broadcast(&pool->cond_has_elements);
    pthread_mutex_unlock(&pool->lock_has_elements);

    for (int i = 0; i < WORKER_COUNT; ++i) {
        int error = pthread_join(pool->workers[i].thread_id, NULL);
    }

    free(pool);
}

void update_world_tick(World* world, UpdateCycle game_time, UpdateWorkerPool* pool) {
    List list;
    { // load world entities
        List* src_list = world_get_entities_to_update(world);
        list_init(&list, src_list->_element_size, WORLD_DEFAULT_ENTITY_LIST_CAPACITY);
        list_swap_contents(&list, src_list);
    }

    int nr_of_entities = list_get_size(&list);
    int elements_per_worker = max_i(nr_of_entities / WORKER_COUNT + 1, 16);
    int next_batch_start = 0;

    pool->state = STATE_PRE_UPDATE;
    while (next_batch_start < nr_of_entities) {
        int end_index = min_i(next_batch_start + elements_per_worker, nr_of_entities);
        ListIterator batch = list_sublist_iterator(&list, next_batch_start, end_index);

        pthread_mutex_lock(&pool->lock_has_space);
        
        while (((pool->buffer_head + 1) % BUFFER_SIZE) == pool->buffer_tail) {
            pthread_cond_wait(&pool->cond_has_space, &pool->lock_has_space);
        }
        
        // set buffer
        int head = pool->buffer_head;
        pool->buffer[head] = batch;
        pool->buffer_head = (head + 1) % BUFFER_SIZE;
        
        pthread_mutex_unlock(&pool->lock_has_space);
        // signal buffer available
        pthread_cond_signal(&pool->cond_has_elements);

        next_batch_start = end_index;
    }

//TODO REPLACE
    pthread_mutex_lock(&pool->lock_has_space);
    while (pool->buffer_head != pool->buffer_tail) {
        pthread_cond_wait(&pool->cond_has_space, &pool->lock_has_space);
    }
    pool->state = STATE_POST_UPDATE;

    pthread_mutex_unlock(&pool->lock_has_space);
}
