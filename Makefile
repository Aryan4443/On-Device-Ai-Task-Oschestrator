CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 -pthread
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDFLAGS = -pthread -lm -lrt
else
    LDFLAGS = -pthread -lm
endif

PYTHON = python3
PYTHON_CONFIG = python3-config

# Directories
SRC_DIR = src
BUILD_DIR = build
PYTHON_DIR = python

# Source files
C_SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/orchestrator.c $(SRC_DIR)/task_queue.c $(SRC_DIR)/thread_pool.c $(SRC_DIR)/resource_monitor.c
C_OBJECTS = $(C_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Targets
TARGET = orchestrator

.PHONY: all clean install test

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(C_OBJECTS)
	$(CC) $(C_OBJECTS) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

install: all
	pip3 install -r requirements.txt

test: all
	$(PYTHON) $(PYTHON_DIR)/test_orchestrator.py

