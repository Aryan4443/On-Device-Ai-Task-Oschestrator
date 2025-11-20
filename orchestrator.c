#include "orchestrator.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

static orchestrator_t *g_orchestrator = NULL;

static void signal_handler(int sig) {
    if (g_orchestrator) {
        printf("\nReceived signal %d, shutting down...\n", sig);
        orchestrator_stop(g_orchestrator);
    }
}

static int python_inference_execute(void *data) {
    // This will be called by worker threads
    // In a real implementation, this would communicate with Python process
    char *task_data = (char*)data;
    if (!task_data) return -1;
    
    // For now, simulate execution
    printf("Executing AI inference task: %s\n", task_data);
    usleep(100000); // Simulate work (100ms)
    
    return 0;
}

static void python_inference_cleanup(void *data) {
    // Cleanup task data if needed
    (void)data; // Suppress unused parameter warning
}

orchestrator_t* orchestrator_create(size_t num_threads, size_t queue_size,
                                    const char *python_script_path) {
    orchestrator_t *orch = (orchestrator_t*)malloc(sizeof(orchestrator_t));
    if (!orch) return NULL;
    
    orch->task_queue = task_queue_create(queue_size);
    if (!orch->task_queue) {
        free(orch);
        return NULL;
    }
    
    orch->thread_pool = thread_pool_create(num_threads, orch->task_queue);
    if (!orch->thread_pool) {
        task_queue_destroy(orch->task_queue);
        free(orch);
        return NULL;
    }
    
    orch->resource_monitor = resource_monitor_create(1000); // Check every second
    if (!orch->resource_monitor) {
        thread_pool_destroy(orch->thread_pool);
        task_queue_destroy(orch->task_queue);
        free(orch);
        return NULL;
    }
    
    if (python_script_path) {
        strncpy(orch->python_script_path, python_script_path, MAX_PYTHON_SCRIPT_PATH - 1);
        orch->python_script_path[MAX_PYTHON_SCRIPT_PATH - 1] = '\0';
    } else {
        strncpy(orch->python_script_path, "python/inference_engine.py", MAX_PYTHON_SCRIPT_PATH - 1);
        orch->python_script_path[MAX_PYTHON_SCRIPT_PATH - 1] = '\0';
    }
    
    orch->running = false;
    orch->num_threads = num_threads;
    orch->queue_size = queue_size;
    
    return orch;
}

int orchestrator_start(orchestrator_t *orch) {
    if (!orch || orch->running) return -1;
    
    // Setup signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    g_orchestrator = orch;
    
    if (thread_pool_start(orch->thread_pool) != 0) {
        return -1;
    }
    
    orch->running = true;
    printf("Orchestrator started with %zu threads\n", orch->num_threads);
    
    return 0;
}

void orchestrator_stop(orchestrator_t *orch) {
    if (!orch || !orch->running) return;
    
    orch->running = false;
    thread_pool_shutdown(orch->thread_pool);
    printf("Orchestrator stopped\n");
}

void orchestrator_destroy(orchestrator_t *orch) {
    if (!orch) return;
    
    orchestrator_stop(orch);
    
    resource_monitor_destroy(orch->resource_monitor);
    thread_pool_destroy(orch->thread_pool);
    task_queue_destroy(orch->task_queue);
    free(orch);
    
    if (g_orchestrator == orch) {
        g_orchestrator = NULL;
    }
}

int orchestrator_submit_task(orchestrator_t *orch, const char *task_id,
                            task_priority_t priority, void *data, size_t data_size) {
    if (!orch || !task_id) return -1;
    
    // Check resource health before submitting
    if (!resource_monitor_is_healthy(orch->resource_monitor, 90.0, 85.0)) {
        printf("Warning: System resources high, task may be delayed\n");
    }
    
    // Allocate and copy task data
    void *task_data = malloc(data_size);
    if (!task_data) return -1;
    memcpy(task_data, data, data_size);
    
    task_t *task = task_create(task_id, priority, task_data, data_size,
                              python_inference_execute,
                              python_inference_cleanup);
    if (!task) {
        free(task_data);
        return -1;
    }
    
    int result = task_queue_enqueue(orch->task_queue, task);
    if (result != 0) {
        task_destroy(task);
        return -1;
    }
    
    printf("Task '%s' submitted with priority %d\n", task_id, priority);
    return 0;
}

bool orchestrator_is_running(orchestrator_t *orch) {
    return orch && orch->running;
}

size_t orchestrator_get_queue_size(orchestrator_t *orch) {
    if (!orch) return 0;
    return task_queue_size(orch->task_queue);
}

