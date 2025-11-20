#include "orchestrator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

void print_usage(const char *program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf("  -t <num>     Number of worker threads (default: 4)\n");
    printf("  -q <size>    Task queue size (default: 100)\n");
    printf("  -p <path>    Path to Python inference script (default: python/inference_engine.py)\n");
    printf("  -i           Interactive mode - submit tasks manually\n");
    printf("  -n           No sample tasks - skip default test tasks\n");
    printf("  -h           Show this help message\n");
}

void submit_sample_tasks(orchestrator_t *orch) {
    printf("\nSubmitting sample tasks...\n");
    
    for (int i = 0; i < 10; i++) {
        char task_id[64];
        snprintf(task_id, sizeof(task_id), "task_%d", i);
        
        char task_data[256];
        snprintf(task_data, sizeof(task_data), "AI inference task %d", i);
        
        task_priority_t priority = (i % 4 == 0) ? TASK_PRIORITY_CRITICAL :
                                   (i % 3 == 0) ? TASK_PRIORITY_HIGH :
                                   (i % 2 == 0) ? TASK_PRIORITY_NORMAL :
                                   TASK_PRIORITY_LOW;
        
        orchestrator_submit_task(orch, task_id, priority, task_data, strlen(task_data) + 1);
        usleep(50000); // 50ms delay between submissions
    }
}

void interactive_mode(orchestrator_t *orch) {
    char line[512];
    char task_id[64];
    char task_data[256];
    int priority;
    
    printf("\n=== Interactive Task Submission Mode ===\n");
    printf("Enter tasks (format: task_id priority data)\n");
    printf("Priority: 0=Low, 1=Normal, 2=High, 3=Critical\n");
    printf("Type 'quit' or 'exit' to stop submitting tasks\n");
    printf("Type 'status' to check queue status\n\n");
    
    while (1) {
        printf("orchestrator> ");
        fflush(stdout);
        
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        if (strlen(line) == 0) {
            continue;
        }
        
        // Check for quit commands
        if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0 || strcmp(line, "q") == 0) {
            printf("Exiting interactive mode...\n");
            break;
        }
        
        // Check for status command
        if (strcmp(line, "status") == 0 || strcmp(line, "s") == 0) {
            size_t qsize = orchestrator_get_queue_size(orch);
            printf("Queue size: %zu tasks\n", qsize);
            continue;
        }
        
        // Parse input: task_id priority data
        if (sscanf(line, "%63s %d %255[^\n]", task_id, &priority, task_data) == 3) {
            if (priority < 0 || priority > 3) {
                printf("Error: Priority must be 0-3\n");
                continue;
            }
            
            task_priority_t task_priority = (task_priority_t)priority;
            if (orchestrator_submit_task(orch, task_id, task_priority, 
                                        task_data, strlen(task_data) + 1) == 0) {
                printf("Task '%s' submitted successfully\n", task_id);
            } else {
                printf("Error: Failed to submit task '%s'\n", task_id);
            }
        } else {
            printf("Error: Invalid format. Use: task_id priority data\n");
            printf("Example: my_task 2 Hello World\n");
        }
    }
}

int main(int argc, char *argv[]) {
    size_t num_threads = DEFAULT_NUM_THREADS;
    size_t queue_size = DEFAULT_QUEUE_SIZE;
    const char *python_path = NULL;
    bool interactive = false;
    bool no_samples = false;
    
    int opt;
    while ((opt = getopt(argc, argv, "t:q:p:inh")) != -1) {
        switch (opt) {
            case 't':
                num_threads = (size_t)atoi(optarg);
                if (num_threads == 0) num_threads = DEFAULT_NUM_THREADS;
                break;
            case 'q':
                queue_size = (size_t)atoi(optarg);
                if (queue_size == 0) queue_size = DEFAULT_QUEUE_SIZE;
                break;
            case 'p':
                python_path = optarg;
                break;
            case 'i':
                interactive = true;
                break;
            case 'n':
                no_samples = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    printf("=== On-Device AI Task Orchestrator ===\n");
    printf("Threads: %zu, Queue Size: %zu\n", num_threads, queue_size);
    
    orchestrator_t *orch = orchestrator_create(num_threads, queue_size, python_path);
    if (!orch) {
        fprintf(stderr, "Failed to create orchestrator\n");
        return 1;
    }
    
    if (orchestrator_start(orch) != 0) {
        fprintf(stderr, "Failed to start orchestrator\n");
        orchestrator_destroy(orch);
        return 1;
    }
    
    // Submit tasks based on mode
    if (interactive) {
        interactive_mode(orch);
    } else if (!no_samples) {
        submit_sample_tasks(orch);
    } else {
        printf("\nNo tasks submitted. Use -i for interactive mode or remove -n for sample tasks.\n");
    }
    
    // Monitor queue and resources
    printf("\nMonitoring orchestrator...\n");
    int empty_count = 0;
    const int EMPTY_THRESHOLD = 3; // Exit after queue is empty for 3 checks (6 seconds)
    
    while (orchestrator_is_running(orch)) {
        size_t queue_size = orchestrator_get_queue_size(orch);
        system_resources_t resources;
        
        if (resource_monitor_get_resources(orch->resource_monitor, &resources) == 0) {
            printf("Queue: %zu tasks | CPU: %.1f%% | Memory: %.1f%% used\n",
                   queue_size,
                   resources.cpu_usage,
                   (double)resources.memory_used / resources.memory_total * 100.0);
        }
        
        if (queue_size == 0) {
            empty_count++;
            if (empty_count >= EMPTY_THRESHOLD) {
                printf("All tasks completed. Exiting...\n");
                break;
            }
            printf("All tasks completed. Waiting %d more check(s)...\n", 
                   EMPTY_THRESHOLD - empty_count);
        } else {
            empty_count = 0; // Reset counter if new tasks arrive
        }
        
        sleep(2);
    }
    
    orchestrator_destroy(orch);
    printf("Orchestrator terminated\n");
    
    return 0;
}

