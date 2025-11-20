#!/usr/bin/env python3
"""
Test script for the AI Task Orchestrator
"""

import sys
import json
import time
from inference_engine import InferenceEngine


def test_inference_engine():
    """Test the inference engine without a model"""
    print("Testing Inference Engine...")
    
    engine = InferenceEngine()
    
    # Test with mock inference
    test_tasks = [
        {
            'task_id': 'test_001',
            'input_data': None
        },
        {
            'task_id': 'test_002',
            'input_data': [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]
        }
    ]
    
    for task in test_tasks:
        print(f"\nProcessing task: {task['task_id']}")
        result = engine.process_task(task)
        print(f"Result: {json.dumps(result, indent=2)}")
        time.sleep(0.1)
    
    print("\nInference engine test completed!")


def test_communication():
    """Test the communication layer"""
    print("\nTesting Communication Layer...")
    
    from communication import CPythonCommunicator
    
    comm = CPythonCommunicator()
    
    test_task = {
        'task_id': 'comm_test_001',
        'priority': 2,
        'input_data': [[1.0, 2.0], [3.0, 4.0]]
    }
    
    if comm.send_task_via_stdin(test_task):
        print("Task sent successfully")
    
    comm.close()
    print("Communication test completed!")


if __name__ == '__main__':
    print("=== AI Task Orchestrator Test Suite ===\n")
    
    if len(sys.argv) > 1 and sys.argv[1] == '--comm':
        test_communication()
    else:
        test_inference_engine()
        test_communication()
    
    print("\nAll tests completed!")

