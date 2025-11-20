#include "resource_monitor.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#ifdef __linux__
#include <sys/sysinfo.h>
#elif __APPLE__
#include <sys/sysctl.h>
#include <mach/mach.h>
#endif

resource_monitor_t* resource_monitor_create(uint32_t check_interval_ms) {
    resource_monitor_t *monitor = (resource_monitor_t*)malloc(sizeof(resource_monitor_t));
    if (!monitor) return NULL;
    
    monitor->monitoring = false;
    monitor->check_interval_ms = check_interval_ms;
    
    return monitor;
}

void resource_monitor_destroy(resource_monitor_t *monitor) {
    if (monitor) {
        free(monitor);
    }
}

int resource_monitor_get_resources(resource_monitor_t *monitor, system_resources_t *resources) {
    if (!monitor || !resources) return -1;
    
    // CPU usage (simplified - in production, calculate over time)
#ifdef __linux__
    FILE *file = fopen("/proc/loadavg", "r");
    if (file) {
        double load[3];
        if (fscanf(file, "%lf %lf %lf", &load[0], &load[1], &load[2]) == 3) {
            resources->cpu_usage = load[0] * 100.0; // Simplified
        } else {
            resources->cpu_usage = 0.0;
        }
        fclose(file);
    } else {
        resources->cpu_usage = 0.0;
    }
#else
    // For macOS and other systems, use a simplified approach
    // In production, use sysctl or other platform-specific APIs
    resources->cpu_usage = 0.0; // Placeholder
#endif
    
    // Memory usage
#ifdef __linux__
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        resources->memory_total = info.totalram * info.mem_unit;
        resources->memory_available = info.freeram * info.mem_unit;
        resources->memory_used = resources->memory_total - resources->memory_available;
    } else {
        resources->memory_total = 0;
        resources->memory_available = 0;
        resources->memory_used = 0;
    }
#elif __APPLE__
    int mib[2];
    uint64_t total_mem;
    size_t len = sizeof(total_mem);
    
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    if (sysctl(mib, 2, &total_mem, &len, NULL, 0) == 0) {
        resources->memory_total = total_mem;
        
        vm_size_t page_size;
        vm_statistics64_data_t vm_stat;
        mach_port_t mach_port = mach_host_self();
        mach_msg_type_number_t count = sizeof(vm_stat) / sizeof(natural_t);
        
        if (host_page_size(mach_port, &page_size) == KERN_SUCCESS &&
            host_statistics64(mach_port, HOST_VM_INFO, (host_info64_t)&vm_stat, &count) == KERN_SUCCESS) {
            resources->memory_available = vm_stat.free_count * page_size;
            resources->memory_used = resources->memory_total - resources->memory_available;
        } else {
            resources->memory_available = 0;
            resources->memory_used = 0;
        }
    } else {
        resources->memory_total = 0;
        resources->memory_available = 0;
        resources->memory_used = 0;
    }
#else
    resources->memory_total = 0;
    resources->memory_available = 0;
    resources->memory_used = 0;
#endif
    
    return 0;
}

bool resource_monitor_is_healthy(resource_monitor_t *monitor, 
                                  double max_cpu_percent, 
                                  double max_memory_percent) {
    if (!monitor) return false;
    
    system_resources_t resources;
    if (resource_monitor_get_resources(monitor, &resources) != 0) {
        return false;
    }
    
    double cpu_percent = resources.cpu_usage;
    double memory_percent = (double)resources.memory_used / resources.memory_total * 100.0;
    
    return (cpu_percent < max_cpu_percent && memory_percent < max_memory_percent);
}

