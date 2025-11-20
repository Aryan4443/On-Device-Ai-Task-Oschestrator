#!/usr/bin/env python3
"""
AI Inference Engine for On-Device Task Orchestrator
Handles model loading, inference execution, and result processing
"""

import sys
import json
import numpy as np
import onnxruntime as ort
from typing import Dict, Any, Optional
import os
import time


class InferenceEngine:
    """Manages AI model loading and inference execution"""
    
    def __init__(self, model_path: Optional[str] = None):
        """
        Initialize the inference engine
        
        Args:
            model_path: Path to ONNX model file (optional, can load later)
        """
        self.model_path = model_path
        self.session = None
        self.input_name = None
        self.output_name = None
        self.model_loaded = False
        
        if model_path and os.path.exists(model_path):
            self.load_model(model_path)
    
    def load_model(self, model_path: str) -> bool:
        """
        Load an ONNX model
        
        Args:
            model_path: Path to ONNX model file
            
        Returns:
            True if model loaded successfully, False otherwise
        """
        try:
            if not os.path.exists(model_path):
                print(f"Error: Model file not found: {model_path}", file=sys.stderr)
                return False
            
            # Create ONNX Runtime session
            sess_options = ort.SessionOptions()
            sess_options.intra_op_num_threads = 1
            sess_options.inter_op_num_threads = 1
            
            self.session = ort.InferenceSession(
                model_path,
                sess_options=sess_options,
                providers=['CPUExecutionProvider']
            )
            
            # Get input/output names
            self.input_name = self.session.get_inputs()[0].name
            self.output_name = self.session.get_outputs()[0].name
            
            self.model_path = model_path
            self.model_loaded = True
            
            print(f"Model loaded successfully: {model_path}")
            return True
            
        except Exception as e:
            print(f"Error loading model: {e}", file=sys.stderr)
            self.model_loaded = False
            return False
    
    def create_dummy_input(self, shape: tuple) -> np.ndarray:
        """
        Create a dummy input tensor for testing
        
        Args:
            shape: Shape of the input tensor
            
        Returns:
            NumPy array with random values
        """
        return np.random.randn(*shape).astype(np.float32)
    
    def run_inference(self, input_data: Optional[np.ndarray] = None) -> Optional[np.ndarray]:
        """
        Run inference on input data
        
        Args:
            input_data: Input tensor (if None, creates dummy input)
            
        Returns:
            Output tensor or None if inference failed
        """
        if not self.model_loaded or self.session is None:
            print("Error: Model not loaded", file=sys.stderr)
            return None
        
        try:
            # Use provided input or create dummy input
            if input_data is None:
                input_shape = self.session.get_inputs()[0].shape
                # Replace dynamic dimensions with fixed values
                input_shape = tuple([1 if dim is None or dim < 0 else dim for dim in input_shape])
                input_data = self.create_dummy_input(input_shape)
            
            # Run inference
            outputs = self.session.run([self.output_name], {self.input_name: input_data})
            
            return outputs[0]
            
        except Exception as e:
            print(f"Error during inference: {e}", file=sys.stderr)
            return None
    
    def process_task(self, task_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Process a task from the orchestrator
        
        Args:
            task_data: Task data dictionary containing task information
            
        Returns:
            Result dictionary with inference output
        """
        start_time = time.time()
        
        # Extract task information
        task_id = task_data.get('task_id', 'unknown')
        input_data = task_data.get('input_data', None)
        
        # Convert input data to numpy array if provided as list
        if input_data is not None and isinstance(input_data, list):
            input_data = np.array(input_data, dtype=np.float32)
        
        # Run inference
        output = self.run_inference(input_data)
        
        inference_time = time.time() - start_time
        
        result = {
            'task_id': task_id,
            'status': 'completed' if output is not None else 'failed',
            'inference_time': inference_time,
            'output_shape': list(output.shape) if output is not None else None,
            'output_sample': output.flatten()[:10].tolist() if output is not None else None
        }
        
        return result


def main():
    """Main entry point for inference engine"""
    engine = InferenceEngine()
    
    # For testing without a model, create a simple mock inference
    if len(sys.argv) > 1:
        model_path = sys.argv[1]
        if not engine.load_model(model_path):
            print("Running in mock mode (no model loaded)")
    else:
        print("Running in mock mode (no model provided)")
    
    # Read task from stdin (JSON format)
    try:
        line = sys.stdin.readline()
        if line:
            task_data = json.loads(line.strip())
            result = engine.process_task(task_data)
            print(json.dumps(result))
        else:
            # Run a test inference
            test_task = {
                'task_id': 'test_task',
                'input_data': None
            }
            result = engine.process_task(test_task)
            print(json.dumps(result))
    except json.JSONDecodeError as e:
        print(f"Error parsing JSON: {e}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()

