#!/usr/bin/env python3
"""
Communication layer for C-Python interprocess communication
Supports shared memory, pipes, and message passing
"""

import json
import sys
import os
import struct
import mmap
import time
from typing import Dict, Any, Optional


class CPythonCommunicator:
    """Handles communication between C orchestrator and Python inference engine"""
    
    def __init__(self, shared_mem_name: str = "/ai_orchestrator_shm", 
                 shared_mem_size: int = 4096):
        """
        Initialize communicator
        
        Args:
            shared_mem_name: Name for shared memory segment
            shared_mem_size: Size of shared memory segment in bytes
        """
        self.shared_mem_name = shared_mem_name
        self.shared_mem_size = shared_mem_size
        self.shared_mem = None
    
    def send_task_via_stdin(self, task_data: Dict[str, Any]) -> bool:
        """
        Send task data via stdin (JSON format)
        
        Args:
            task_data: Task data dictionary
            
        Returns:
            True if sent successfully
        """
        try:
            json_data = json.dumps(task_data)
            print(json_data, flush=True)
            return True
        except Exception as e:
            print(f"Error sending task: {e}", file=sys.stderr)
            return False
    
    def receive_result_via_stdout(self) -> Optional[Dict[str, Any]]:
        """
        Receive result data via stdout (JSON format)
        
        Returns:
            Result dictionary or None if failed
        """
        try:
            line = sys.stdin.readline()
            if line:
                return json.loads(line.strip())
        except json.JSONDecodeError as e:
            print(f"Error parsing result: {e}", file=sys.stderr)
        except Exception as e:
            print(f"Error receiving result: {e}", file=sys.stderr)
        
        return None
    
    def create_shared_memory(self) -> bool:
        """
        Create shared memory segment (Unix only)
        
        Returns:
            True if created successfully
        """
        try:
            # This is a simplified version
            # In production, use proper shared memory with semaphores
            self.shared_mem = mmap.mmap(-1, self.shared_mem_size)
            return True
        except Exception as e:
            print(f"Error creating shared memory: {e}", file=sys.stderr)
            return False
    
    def write_to_shared_memory(self, data: bytes) -> bool:
        """
        Write data to shared memory
        
        Args:
            data: Data to write
            
        Returns:
            True if written successfully
        """
        if not self.shared_mem:
            return False
        
        try:
            if len(data) > self.shared_mem_size:
                return False
            
            self.shared_mem.seek(0)
            self.shared_mem.write(data)
            return True
        except Exception as e:
            print(f"Error writing to shared memory: {e}", file=sys.stderr)
            return False
    
    def read_from_shared_memory(self, size: int) -> Optional[bytes]:
        """
        Read data from shared memory
        
        Args:
            size: Number of bytes to read
            
        Returns:
            Data bytes or None if failed
        """
        if not self.shared_mem:
            return None
        
        try:
            self.shared_mem.seek(0)
            return self.shared_mem.read(size)
        except Exception as e:
            print(f"Error reading from shared memory: {e}", file=sys.stderr)
            return None
    
    def close(self):
        """Close communication channels"""
        if self.shared_mem:
            self.shared_mem.close()
            self.shared_mem = None


def main():
    """Test communication layer"""
    comm = CPythonCommunicator()
    
    # Test stdin/stdout communication
    test_task = {
        'task_id': 'test_001',
        'priority': 2,
        'input_data': [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]
    }
    
    comm.send_task_via_stdin(test_task)
    result = comm.receive_result_via_stdout()
    
    if result:
        print(f"Received result: {result}")
    else:
        print("No result received")
    
    comm.close()


if __name__ == '__main__':
    main()

