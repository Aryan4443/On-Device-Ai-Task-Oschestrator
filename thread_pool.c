#include "thread_pool.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

static void* worker_thread(void *arg) {
    thread_pool_t *pool = (thread_pool_t*)arg;
    
    while (true) {
        pthread_mutex_lock(&pool->mutex);
        
        while (task_queue_is_empty(pool->task_queue) && !pool->shutdown) {
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }
        
        if (pool->shutdown && task_queue_is_empty(pool->task_queue)) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }
        
        pthread_mutex_unlock(&pool->mutex);
        
        task_t *task = task_queue_dequeue(pool->task_queue);
        if (task) {
            task->status = TASK_STATUS_RUNNING;
            
            if (task->execute_callback) {
                int result = task->execute_callback(task->data);
                task->status = (result == 0) ? TASK_STATUS_COMPLETED : TASK_STATUS_FAILED;
            } else {
                task->status = TASK_STATUS_FAILED;
            }
            
            task_destroy(task);
        }
    }
    
    return NULL;
}

thread_pool_t* thread_pool_create(size_t num_threads, task_queue_t *queue) {
    if (num_threads == 0 || !queue) return NULL;
    
    thread_pool_t *pool = (thread_pool_t*)malloc(sizeof(thread_pool_t));
    if (!pool) return NULL;
    
    pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * num_threads);
    if (!pool->threads) {
        free(pool);
        return NULL;
    }
    
    pool->num_threads = num_threads;
    pool->task_queue = queue;
    pool->shutdown = false;
    
    if (pthread_mutex_init(&pool->mutex, NULL) != 0) {
        free(pool->threads);
        free(pool);
        return NULL;
    }
    
    if (pthread_cond_init(&pool->cond, NULL) != 0) {
        pthread_mutex_destroy(&pool->mutex);
        free(pool->threads);
        free(pool);
        return NULL;
    }
    
    return pool;
}

int thread_pool_start(thread_pool_t *pool) {
    if (!pool) return -1;
    
    for (size_t i = 0; i < pool->num_threads; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker_thread, pool) != 0) {
            // Cleanup already created threads
            pool->shutdown = true;
            pthread_cond_broadcast(&pool->cond);
            for (size_t j = 0; j < i; j++) {
                pthread_join(pool->threads[j], NULL);
            }
            return -1;
        }
    }
    
    return 0;
}

void thread_pool_shutdown(thread_pool_t *pool) {
    if (!pool || pool->shutdown) return;
    
    pthread_mutex_lock(&pool->mutex);
    pool->shutdown = true;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);
    
    for (size_t i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }
}

void thread_pool_destroy(thread_pool_t *pool) {
    if (!pool) return;
    
    thread_pool_shutdown(pool);
    
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond);
    free(pool->threads);
    free(pool);
}

bool thread_pool_is_shutdown(thread_pool_t *pool) {
    if (!pool) return true;
    
    pthread_mutex_lock(&pool->mutex);
    bool shutdown = pool->shutdown;
    pthread_mutex_unlock(&pool->mutex);
    
    return shutdown;
}

