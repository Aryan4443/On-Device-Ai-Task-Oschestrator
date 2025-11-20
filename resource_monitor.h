#ifndef RESOURCE_MONITOR_H
#define RESOURCE_MONITOR_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    double cpu_usage;
    uint64_t memory_used;
    uint64_t memory_total;
    uint64_t memory_available;
} system_resources_t;

typedef struct {
    bool monitoring;
    uint32_t check_interval_ms;
} resource_monitor_t;

resource_monitor_t* resource_monitor_create(uint32_t check_interval_ms);
void resource_monitor_destroy(resource_monitor_t *monitor);
int resource_monitor_get_resources(resource_monitor_t *monitor, system_resources_t *resources);
bool resource_monitor_is_healthy(resource_monitor_t *monitor, double max_cpu_percent, double max_memory_percent);

#endif // RESOURCE_MONITOR_H

