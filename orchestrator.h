#ifndef ORCHESTRATOR_H
#define ORCHESTRATOR_H

#include "task_queue.h"
#include "thread_pool.h"
#include "resource_monitor.h"
#include <stdbool.h>

#define MAX_PYTHON_SCRIPT_PATH 256
#define DEFAULT_NUM_THREADS 4
#define DEFAULT_QUEUE_SIZE 100

typedef struct {
    task_queue_t *task_queue;
    thread_pool_t *thread_pool;
    resource_monitor_t *resource_monitor;
    char python_script_path[MAX_PYTHON_SCRIPT_PATH];
    bool running;
    size_t num_threads;
    size_t queue_size;
} orchestrator_t;

orchestrator_t* orchestrator_create(size_t num_threads, size_t queue_size, 
                                    const char *python_script_path);
void orchestrator_destroy(orchestrator_t *orch);
int orchestrator_start(orchestrator_t *orch);
void orchestrator_stop(orchestrator_t *orch);
int orchestrator_submit_task(orchestrator_t *orch, const char *task_id,
                             task_priority_t priority, void *data, size_t data_size);
bool orchestrator_is_running(orchestrator_t *orch);
size_t orchestrator_get_queue_size(orchestrator_t *orch);

#endif // ORCHESTRATOR_H

