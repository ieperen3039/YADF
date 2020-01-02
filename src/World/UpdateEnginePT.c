//
// Created by s152717 on 13-12-2019.
//

#include "UpdateEngine.h"
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#ifndef WORKER_COUNT
#define WORKER_COUNT 7
#endif

struct _UpdateWorkerPool {
    struct {
        pthread_t thread_id;
    } workers[WORKER_COUNT];

    ListIterator buffer;
    sem_t buffer_guard; // whether queue contains critical data
    pthread_mutex_t condition_mutex;
    pthread_cond_t condition;
    volatile char keep_going;
};

void* worker_loop(void* worker_pool_ptr) {
    UpdateWorkerPool* pool = worker_pool_ptr;
    ListIterator entities;

    while (pool->keep_going) {
        pthread_mutex_lock(&pool->condition_mutex);

        pthread_cond_wait(&pool->condition, &pool->condition_mutex);

        entities = pool->buffer;
        pool->buffer = list_iterator_empty();
        // set buffer available
        sem_post(&pool->buffer_guard);
        pthread_mutex_unlock(&pool->condition_mutex);

        while (list_iterator_has_next(&entities)) {
            Entity** ety = list_iterator_next(&entities);
            entity_update(*ety);
        }
    }

    return NULL;
}

UpdateWorkerPool* update_workers_new() {
    UpdateWorkerPool* pool = malloc(sizeof(UpdateWorkerPool));
    pool->keep_going = 1;
    sem_init(&pool->buffer_guard, 1, 0);
    pthread_mutex_init(&pool->condition_mutex, NULL);
    pthread_cond_init(&pool->condition, NULL);

    for (int i = 0; i < WORKER_COUNT; ++i) {
        pthread_create(&pool->workers[i].thread_id, NULL, worker_loop, pool);
    }

    return pool;
}

void update_workers_free(UpdateWorkerPool* pool) {
    pthread_mutex_lock(&pool->condition_mutex);
    pool->keep_going = 0;
    pthread_cond_broadcast(&pool->condition);
    pthread_mutex_unlock(&pool->condition_mutex);

    for (int i = 0; i < WORKER_COUNT; ++i) {
        int error = pthread_join(pool->workers[i].thread_id, NULL);
    }

    free(pool);
}

void update_world_tick(World* world, UpdateCycle game_time, UpdateWorkerPool* worker_pool) {
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

        // wait until buffer is available
        sem_wait(&worker_pool->buffer_guard);
        // set buffer
        worker_pool->buffer = batch;
        // signal buffer available
        pthread_cond_signal(&worker_pool->condition);

        next_batch_start = end_index;
    }
}
