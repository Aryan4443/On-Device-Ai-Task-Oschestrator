#include "task_queue.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

task_queue_t* task_queue_create(size_t max_size) {
    task_queue_t *queue = (task_queue_t*)malloc(sizeof(task_queue_t));
    if (!queue) return NULL;
    
    queue->head = NULL;
    queue->size = 0;
    queue->max_size = max_size;
    
    if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
        free(queue);
        return NULL;
    }
    
    if (pthread_cond_init(&queue->cond, NULL) != 0) {
        pthread_mutex_destroy(&queue->mutex);
        free(queue);
        return NULL;
    }
    
    return queue;
}

void task_queue_destroy(task_queue_t *queue) {
    if (!queue) return;
    
    pthread_mutex_lock(&queue->mutex);
    
    task_node_t *current = queue->head;
    while (current) {
        task_node_t *next = current->next;
        if (current->task) {
            task_destroy(current->task);
        }
        free(current);
        current = next;
    }
    
    pthread_mutex_unlock(&queue->mutex);
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
    free(queue);
}

static task_node_t* create_node(task_t *task) {
    task_node_t *node = (task_node_t*)malloc(sizeof(task_node_t));
    if (!node) return NULL;
    node->task = task;
    node->next = NULL;
    return node;
}

static void insert_sorted(task_queue_t *queue, task_node_t *new_node) {
    if (!queue->head || new_node->task->priority > queue->head->task->priority) {
        new_node->next = queue->head;
        queue->head = new_node;
        return;
    }
    
    task_node_t *current = queue->head;
    while (current->next && 
           current->next->task->priority >= new_node->task->priority) {
        current = current->next;
    }
    
    new_node->next = current->next;
    current->next = new_node;
}

int task_queue_enqueue(task_queue_t *queue, task_t *task) {
    if (!queue || !task) return -1;
    
    pthread_mutex_lock(&queue->mutex);
    
    if (queue->size >= queue->max_size) {
        pthread_mutex_unlock(&queue->mutex);
        return -1; // Queue full
    }
    
    task_node_t *node = create_node(task);
    if (!node) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }
    
    insert_sorted(queue, node);
    queue->size++;
    
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
    
    return 0;
}

task_t* task_queue_dequeue(task_queue_t *queue) {
    if (!queue) return NULL;
    
    pthread_mutex_lock(&queue->mutex);
    
    while (queue->size == 0) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }
    
    if (!queue->head) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }
    
    task_node_t *node = queue->head;
    queue->head = node->next;
    queue->size--;
    
    task_t *task = node->task;
    free(node);
    
    pthread_mutex_unlock(&queue->mutex);
    return task;
}

task_t* task_queue_peek(task_queue_t *queue) {
    if (!queue) return NULL;
    
    pthread_mutex_lock(&queue->mutex);
    task_t *task = queue->head ? queue->head->task : NULL;
    pthread_mutex_unlock(&queue->mutex);
    
    return task;
}

bool task_queue_is_empty(task_queue_t *queue) {
    if (!queue) return true;
    
    pthread_mutex_lock(&queue->mutex);
    bool empty = (queue->size == 0);
    pthread_mutex_unlock(&queue->mutex);
    
    return empty;
}

bool task_queue_is_full(task_queue_t *queue) {
    if (!queue) return false;
    
    pthread_mutex_lock(&queue->mutex);
    bool full = (queue->size >= queue->max_size);
    pthread_mutex_unlock(&queue->mutex);
    
    return full;
}

size_t task_queue_size(task_queue_t *queue) {
    if (!queue) return 0;
    
    pthread_mutex_lock(&queue->mutex);
    size_t size = queue->size;
    pthread_mutex_unlock(&queue->mutex);
    
    return size;
}

task_t* task_create(const char *task_id, task_priority_t priority,
                    void *data, size_t data_size,
                    int (*execute_callback)(void *),
                    void (*cleanup_callback)(void *)) {
    if (!task_id) return NULL;
    
    task_t *task = (task_t*)malloc(sizeof(task_t));
    if (!task) return NULL;
    
    strncpy(task->task_id, task_id, MAX_TASK_ID_LEN - 1);
    task->task_id[MAX_TASK_ID_LEN - 1] = '\0';
    task->priority = priority;
    task->status = TASK_STATUS_PENDING;
    task->data = data;
    task->data_size = data_size;
    task->execute_callback = execute_callback;
    task->cleanup_callback = cleanup_callback;
    task->timestamp = (uint64_t)time(NULL);
    
    return task;
}

void task_destroy(task_t *task) {
    if (!task) return;
    
    if (task->cleanup_callback && task->data) {
        task->cleanup_callback(task->data);
    }
    
    if (task->data) {
        free(task->data);
    }
    
    free(task);
}

