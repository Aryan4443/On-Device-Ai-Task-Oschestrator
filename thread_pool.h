#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "task_queue.h"
#include <pthread.h>
#include <stdbool.h>

typedef struct {
    pthread_t *threads;
    size_t num_threads;
    task_queue_t *task_queue;
    bool shutdown;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} thread_pool_t;

thread_pool_t* thread_pool_create(size_t num_threads, task_queue_t *queue);
void thread_pool_destroy(thread_pool_t *pool);
int thread_pool_start(thread_pool_t *pool);
void thread_pool_shutdown(thread_pool_t *pool);
bool thread_pool_is_shutdown(thread_pool_t *pool);

#endif // THREAD_POOL_H

