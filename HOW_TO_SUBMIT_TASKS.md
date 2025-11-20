# How to Submit Tasks

There are **three ways** to submit tasks to the orchestrator:

## 1. Interactive Mode (Recommended for Testing)

Run the orchestrator with the `-i` flag to enter interactive mode:

```bash
./orchestrator -i
```

Then you can submit tasks interactively:

```
orchestrator> my_task 2 Hello World
Task 'my_task' submitted successfully

orchestrator> urgent_task 3 Critical data
Task 'urgent_task' submitted successfully

orchestrator> status
Queue size: 2 tasks

orchestrator> quit
```

**Format:** `task_id priority data`
- `task_id`: Unique identifier for the task
- `priority`: 0=Low, 1=Normal, 2=High, 3=Critical
- `data`: Task data (can contain spaces)

**Commands:**
- `quit` or `exit` or `q`: Stop submitting tasks
- `status` or `s`: Check current queue size

## 2. Default Sample Tasks

By default, the orchestrator submits 10 sample tasks automatically:

```bash
./orchestrator
```

This will:
- Submit 10 test tasks with different priorities
- Monitor and process them
- Exit when all tasks complete

## 3. Modify the Code (For Custom Applications)

Edit `src/main.c` and modify the `submit_sample_tasks()` function (around line 60-77):

```c
void submit_sample_tasks(orchestrator_t *orch) {
    // Submit your custom tasks here
    char task_data[] = "Your custom task data";
    orchestrator_submit_task(orch, "my_task_1", TASK_PRIORITY_HIGH, 
                            task_data, strlen(task_data) + 1);
    
    // Add more tasks as needed...
}
```

Or add tasks directly in your code:

```c
orchestrator_t *orch = orchestrator_create(4, 100, NULL);
orchestrator_start(orch);

// Submit your tasks
char data[] = "My task data";
orchestrator_submit_task(orch, "task_001", TASK_PRIORITY_NORMAL,
                        data, strlen(data) + 1);

// ... rest of your code
```

## Task Priority Levels

- **0 (TASK_PRIORITY_LOW)**: Low priority - processed last
- **1 (TASK_PRIORITY_NORMAL)**: Normal priority - default
- **2 (TASK_PRIORITY_HIGH)**: High priority - processed before normal
- **3 (TASK_PRIORITY_CRITICAL)**: Critical priority - processed first

## Examples

### Submit a high-priority task interactively:
```
orchestrator> image_classify 2 /path/to/image.jpg
```

### Submit a critical task:
```
orchestrator> emergency_inference 3 urgent_data_here
```

### Check queue status:
```
orchestrator> status
Queue size: 5 tasks
```

### Run without sample tasks (just monitoring):
```bash
./orchestrator -n
```

### Run in interactive mode with custom settings:
```bash
./orchestrator -i -t 8 -q 200
```

## Where Tasks Are Processed

Tasks are executed by worker threads in the thread pool. Each task:
1. Gets dequeued from the priority queue
2. Executes via the `python_inference_execute()` callback
3. Currently prints the task data (you can modify this in `src/orchestrator.c`)

To integrate with actual AI inference, modify the `python_inference_execute()` function in `src/orchestrator.c` to communicate with your Python inference engine.

