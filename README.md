# On-Device AI Task Orchestrator

A high-performance task orchestrator for managing AI inference tasks on-device, built with C and Python, featuring multithreading, resource management, and efficient task scheduling.

## Architecture

- **C Core**: Low-level orchestration, thread pool management, and task queue
- **Python Engine**: AI model loading and inference execution
- **Communication Layer**: Efficient IPC between C and Python components
- **Scheduler**: Priority-based task scheduling with resource awareness

## Features

- ✅ Multithreaded task execution with configurable thread pool
- ✅ Priority-based task scheduling (Low, Normal, High, Critical)
- ✅ Resource monitoring and management (CPU, Memory)
- ✅ Efficient C-Python interprocess communication
- ✅ UNIX signal handling (SIGINT, SIGTERM)
- ✅ Extensible AI inference framework (ONNX Runtime)
- ✅ Cross-platform support (Linux, macOS)

## Project Structure

```
.
├── src/                    # C source files
│   ├── main.c             # Main entry point
│   ├── orchestrator.c      # Orchestrator core
│   ├── task_queue.c        # Priority queue implementation
│   ├── thread_pool.c       # Thread pool management
│   └── resource_monitor.c  # System resource monitoring
├── python/                 # Python components
│   ├── inference_engine.py    # AI inference engine
│   ├── communication.py        # C-Python communication
│   └── test_orchestrator.py    # Test suite
├── Makefile               # Build configuration
├── requirements.txt       # Python dependencies
└── README.md             # This file
```

## Building

### Prerequisites

- C compiler (GCC/Clang)
- Python 3.8+
- pthread library (standard on Unix systems)
- ONNX Runtime (for AI inference)

### Build Steps

1. Install Python dependencies:
```bash
pip3 install -r requirements.txt
```

2. Build the orchestrator:
```bash
make
```

3. (Optional) Run tests:
```bash
make test
```

## Usage

### Basic Usage

Run the orchestrator with default settings:
```bash
./orchestrator
```

### Command Line Options

```bash
./orchestrator [options]

Options:
  -t <num>     Number of worker threads (default: 4)
  -q <size>    Task queue size (default: 100)
  -p <path>    Path to Python inference script (default: python/inference_engine.py)
  -h           Show help message
```

### Examples

Run with 8 threads and queue size of 200:
```bash
./orchestrator -t 8 -q 200
```

Specify custom Python inference script:
```bash
./orchestrator -p /path/to/custom/inference.py
```

### Task Priorities

Tasks can be submitted with different priorities:
- `TASK_PRIORITY_LOW` (0): Low priority tasks
- `TASK_PRIORITY_NORMAL` (1): Normal priority tasks
- `TASK_PRIORITY_HIGH` (2): High priority tasks
- `TASK_PRIORITY_CRITICAL` (3): Critical priority tasks (executed first)

## Python Inference Engine

The Python inference engine supports ONNX models. Example usage:

```python
from inference_engine import InferenceEngine

# Load a model
engine = InferenceEngine("path/to/model.onnx")

# Run inference
task_data = {
    'task_id': 'task_001',
    'input_data': [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]
}

result = engine.process_task(task_data)
print(result)
```

## Resource Monitoring

The orchestrator monitors system resources and can throttle task submission when:
- CPU usage exceeds 90%
- Memory usage exceeds 85%

These thresholds can be adjusted in `src/orchestrator.c`.

## Thread Safety

All components are thread-safe:
- Task queue uses mutexes and condition variables
- Thread pool properly synchronizes worker threads
- Resource monitoring is safe for concurrent access

## Signal Handling

The orchestrator handles UNIX signals gracefully:
- `SIGINT` (Ctrl+C): Graceful shutdown
- `SIGTERM`: Graceful shutdown

## Extending the Orchestrator

### Adding Custom Task Types

1. Implement your task execution callback in C
2. Create task data structure
3. Submit tasks using `orchestrator_submit_task()`

### Integrating Custom AI Models

1. Load your model in `python/inference_engine.py`
2. Implement custom preprocessing/postprocessing
3. Update the task data format as needed

## Performance Considerations

- Adjust thread count based on CPU cores
- Set appropriate queue size for your workload
- Monitor resource usage to prevent system overload
- Use priority queues for time-sensitive tasks

## Troubleshooting

### Build Issues

- **librt not found (macOS)**: The Makefile automatically handles this
- **pthread errors**: Ensure pthread library is available

### Runtime Issues

- **High CPU usage**: Reduce thread count or increase queue size
- **Memory issues**: Monitor memory usage and adjust task data sizes
- **Python import errors**: Install dependencies with `pip3 install -r requirements.txt`

## License

This project is provided as-is for educational and development purposes.

## Contributing

Feel free to extend and modify this orchestrator for your specific use cases!

