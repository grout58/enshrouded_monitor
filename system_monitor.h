#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <stdint.h>

typedef struct {
    double cpu_percent;
    uint64_t total_mem_kb;
    uint64_t used_mem_kb;
    uint64_t free_mem_kb;
    uint64_t total_swap_kb;
    uint64_t used_swap_kb;
} system_stats_t;

typedef struct {
    uint64_t user;
    uint64_t nice;
    uint64_t system;
    uint64_t idle;
    uint64_t iowait;
    uint64_t irq;
    uint64_t softirq;
    uint64_t steal;
} cpu_times_t;

// Initialize system monitoring
int system_monitor_init(void);

// Get current system statistics
int system_monitor_get_stats(system_stats_t *stats);

// Calculate CPU usage percentage
double system_monitor_calc_cpu_percent(void);

// Get memory information
int system_monitor_get_memory(system_stats_t *stats);

// Cleanup system monitoring resources
void system_monitor_cleanup(void);

#endif // SYSTEM_MONITOR_H
