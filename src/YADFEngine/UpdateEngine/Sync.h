//
// Created by s152717 on 15-1-2020.
//

#ifndef YADF_SYNC_H
#define YADF_SYNC_H

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#if defined(__WINDOWS__)
// required for condition variables
#if _WIN32_WINNT < 0x0501
    #undef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
#endif

#include <process.h>
#include <windows.h>

typedef CRITICAL_SECTION sync_mutex;
typedef HANDLE sync_semaphore;
typedef CONDITION_VARIABLE sync_condition;
typedef unsigned long sync_thread_id;

#elif defined(__linux__)

#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

typedef pthread_mutex_t sync_mutex;
typedef pthread_cond_t sync_condition;
typedef sem_t sync_semaphore;
typedef pthread_t sync_thread_id;

#else
#error Sync: Unsupported platform
#endif

typedef void* (*WorkerFunction)(void* data);

static inline sync_semaphore sync_semaphore_new(int initial_count, int maximum_count);
static inline sync_mutex sync_mutex_new(void);
static inline sync_condition sync_condition_new(void);

static inline sync_thread_id sync_new_thread(WorkerFunction func, void* data);

static inline void sync_lock(sync_mutex* mutex);
static inline void sync_unlock(sync_mutex* mutex);

static inline void sync_condition_wait(sync_condition* condition, sync_mutex* mutex);
static inline void sync_condition_signal(sync_condition* condition);
static inline void sync_condition_broadcast(sync_condition* condition);

static inline void sync_semaphore_wait(sync_semaphore* sem);
static inline bool sync_semaphore_trywait(sync_semaphore* sem);
static inline void sync_semaphore_post(sync_semaphore* sem);


#if defined(__WINDOWS__)

static inline sync_semaphore sync_semaphore_new(int initial_count, int maximum_count) {
    return CreateSemaphore(NULL, initial_count, maximum_count, NULL);
}

static inline sync_mutex sync_mutex_new(){
    sync_mutex m;
    InitializeCriticalSection(&m);
    return m;
}

static inline sync_condition sync_condition_new(){
    sync_condition c;
    InitializeConditionVariable(&c);
    return c;
}

struct __sync_wfunc_data {
    void*(*func)(void*);
    void* data;
};

/** workaround for return function (required to prevent UB) frees data after use */
void __sync_wfunc(void* data){
    struct __sync_wfunc_data* call = data;
    (void) call->func(call->data);
    free(call);
}

static inline sync_thread_id sync_new_thread(WorkerFunction func, void* data) {
    struct __sync_wfunc_data* wfunc = malloc(sizeof(struct __sync_wfunc_data));
    wfunc->func = func;
    wfunc->data = data;
    
    unsigned long thread_handle = _beginthread(__sync_wfunc, 1, wfunc);
    assert(thread_handle >= 0);
    
    return thread_handle;
}

static inline void sync_lock(sync_mutex* m){
    EnterCriticalSection(m);
}

static inline void sync_unlock(sync_mutex* m){
    LeaveCriticalSection(m);
}

static inline void sync_condition_wait(sync_condition* c, sync_mutex* m) {
    SleepConditionVariableCS(c, m, INFINITE);
}

static inline void sync_condition_signal(sync_condition* c) {
    WakeConditionVariable(c);
}

void sync_condition_broadcast(sync_condition* condition) {
    WakeAllConditionVariable(condition);
}

void sync_semaphore_wait(sync_semaphore* s) {
    WaitForSingleObject(*s, INFINITE);
}

bool sync_semaphore_trywait(HANDLE* sem) {
    DWORD result = WaitForSingleObject(*sem, 0);
    return result == WAIT_TIMEOUT ? false : true;
}

static inline void sync_semaphore_post(sync_semaphore* s) {
    ReleaseSemaphore(*s, 1, NULL);
}

#endif

#if defined(__linux__) // TODO

static inline sync_semaphore sync_semaphore_new(int initial_count, int maximum_count);
static inline sync_mutex sync_mutex_new();
static inline sync_condition sync_condition_new();

static inline sync_thread_id sync_new_thread(void* data);

static inline void sync_lock(sync_mutex* mutex);
static inline void sync_unlock(sync_mutex* mutex);

static inline void sync_condition_wait(sync_condition* condition, sync_mutex* mutex);
static inline void sync_condition_signal(sync_condition* condition);

static inline void sync_semaphore_wait(sync_semaphore* s);
static inline void sync_semaphore_post(sync_semaphore* s);

#endif

#endif //YADF_SYNC_H
