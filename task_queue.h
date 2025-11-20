#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_TASK_ID_LEN 64
#define MAX_TASK_DATA_SIZE 4096

typedef enum {
    TASK_PRIORITY_LOW = 0,
    TASK_PRIORITY_NORMAL = 1,
    TASK_PRIORITY_HIGH = 2,
    TASK_PRIORITY_CRITICAL = 3
} task_priority_t;

typedef enum {
    TASK_STATUS_PENDING,
    TASK_STATUS_RUNNING,
    TASK_STATUS_COMPLETED,
    TASK_STATUS_FAILED
} task_status_t;

typedef struct {
    char task_id[MAX_TASK_ID_LEN];
    task_priority_t priority;
    task_status_t status;
    void *data;
    size_t data_size;
    uint64_t timestamp;
    int (*execute_callback)(void *data);
    void (*cleanup_callback)(void *data);
} task_t;

typedef struct task_node {
    task_t *task;
    struct task_node *next;
} task_node_t;

typedef struct {
    task_node_t *head;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    size_t size;
    size_t max_size;
} task_queue_t;

// Queue operations
task_queue_t* task_queue_create(size_t max_size);
void task_queue_destroy(task_queue_t *queue);
int task_queue_enqueue(task_queue_t *queue, task_t *task);
task_t* task_queue_dequeue(task_queue_t *queue);
task_t* task_queue_peek(task_queue_t *queue);
bool task_queue_is_empty(task_queue_t *queue);
bool task_queue_is_full(task_queue_t *queue);
size_t task_queue_size(task_queue_t *queue);

// Task operations
task_t* task_create(const char *task_id, task_priority_t priority, 
                    void *data, size_t data_size,
                    int (*execute_callback)(void *),
                    void (*cleanup_callback)(void *));
void task_destroy(task_t *task);

#endif // TASK_QUEUE_H

