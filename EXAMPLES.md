# Usage Examples

## Example 1: Basic Task Submission

```c
#include "orchestrator.h"

int main() {
    // Create orchestrator with 4 threads and queue size 100
    orchestrator_t *orch = orchestrator_create(4, 100, NULL);

    // Start the orchestrator
    orchestrator_start(orch);

    // Submit a task
    char task_data[] = "Hello, AI!";
    orchestrator_submit_task(orch, "task_001", TASK_PRIORITY_NORMAL,
                             task_data, strlen(task_data) + 1);

    // Wait for tasks to complete
    sleep(5);

    // Stop and cleanup
    orchestrator_stop(orch);
    orchestrator_destroy(orch);

    return 0;
}
```

## Example 2: High Priority Tasks

```c
// Submit critical priority task
char critical_data[] = "Urgent inference request";
orchestrator_submit_task(orch, "critical_001", TASK_PRIORITY_CRITICAL,
                         critical_data, strlen(critical_data) + 1);

// Submit normal priority task
char normal_data[] = "Regular inference request";
orchestrator_submit_task(orch, "normal_001", TASK_PRIORITY_NORMAL,
                         normal_data, strlen(normal_data) + 1);
// Critical task will execute first
```

## Example 3: Python Inference with Custom Model

```python
from inference_engine import InferenceEngine
import numpy as np

# Load your ONNX model
engine = InferenceEngine("models/my_model.onnx")

# Prepare input data
input_data = np.random.randn(1, 3, 224, 224).astype(np.float32)

# Create task
task = {
    'task_id': 'image_classification_001',
    'input_data': input_data.tolist()
}

# Process task
result = engine.process_task(task)
print(f"Inference completed in {result['inference_time']:.3f}s")
print(f"Output shape: {result['output_shape']}")
```

## Example 4: Resource-Aware Task Submission

```c
system_resources_t resources;
resource_monitor_get_resources(orch->resource_monitor, &resources);

// Check if system is healthy before submitting
if (resource_monitor_is_healthy(orch->resource_monitor, 80.0, 75.0)) {
    // System is healthy, submit task
    orchestrator_submit_task(orch, "task_001", TASK_PRIORITY_NORMAL,
                             data, data_size);
} else {
    // System is under load, wait or reduce priority
    printf("System under load, deferring task\n");
}
```

## Example 5: Batch Task Submission

```c
// Submit multiple tasks in a batch
for (int i = 0; i < 100; i++) {
    char task_id[64];
    snprintf(task_id, sizeof(task_id), "batch_task_%d", i);

    char task_data[256];
    snprintf(task_data, sizeof(task_data), "Batch task %d data", i);

    task_priority_t priority = (i % 10 == 0) ? TASK_PRIORITY_HIGH :
                               TASK_PRIORITY_NORMAL;

    orchestrator_submit_task(orch, task_id, priority,
                             task_data, strlen(task_data) + 1);

    // Small delay to avoid overwhelming the queue
    usleep(10000); // 10ms
}
```

## Example 6: Custom Task Execution Callback

```c
int my_custom_execute(void *data) {
    my_task_data_t *task_data = (my_task_data_t*)data;

    // Custom execution logic
    printf("Processing task: %s\n", task_data->task_name);

    // Perform work...

    return 0; // Success
}

void my_custom_cleanup(void *data) {
    my_task_data_t *task_data = (my_task_data_t*)data;
    // Cleanup resources
    free(task_data->buffer);
}

// Create task with custom callbacks
my_task_data_t *data = malloc(sizeof(my_task_data_t));
// ... initialize data ...

task_t *task = task_create("custom_task", TASK_PRIORITY_HIGH,
                           data, sizeof(my_task_data_t),
                           my_custom_execute,
                           my_custom_cleanup);
```

## Example 7: Monitoring Queue Status

```c
while (orchestrator_is_running(orch)) {
    size_t queue_size = orchestrator_get_queue_size(orch);

    if (queue_size > 50) {
        printf("Warning: Queue is getting full (%zu tasks)\n", queue_size);
    }

    if (queue_size == 0) {
        printf("All tasks completed\n");
        break;
    }

    sleep(1);
}
```

## Example 8: Graceful Shutdown

```c
// Setup signal handler for graceful shutdown
void shutdown_handler(int sig) {
    printf("Shutting down gracefully...\n");
    orchestrator_stop(orch);
}

signal(SIGINT, shutdown_handler);
signal(SIGTERM, shutdown_handler);

// Main loop
orchestrator_start(orch);

// ... submit tasks ...

// Wait for completion or signal
while (orchestrator_is_running(orch)) {
    sleep(1);
}

orchestrator_destroy(orch);
```
