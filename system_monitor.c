#include "system_monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static cpu_times_t prev_cpu_times = {0};
static int initialized = 0;

// Read CPU times from /proc/stat
static int read_cpu_times(cpu_times_t *times) {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        return -1;
    }

    char buffer[256];
    if (!fgets(buffer, sizeof(buffer), fp)) {
        fclose(fp);
        return -1;
    }

    // Parse: cpu  user nice system idle iowait irq softirq steal
    int parsed = sscanf(buffer, "cpu %lu %lu %lu %lu %lu %lu %lu %lu",
                       &times->user, &times->nice, &times->system, &times->idle,
                       &times->iowait, &times->irq, &times->softirq, &times->steal);

    fclose(fp);
    return (parsed >= 4) ? 0 : -1;
}

int system_monitor_init(void) {
    if (initialized) {
        return 0;
    }

    // Read initial CPU times
    if (read_cpu_times(&prev_cpu_times) < 0) {
        return -1;
    }

    initialized = 1;
    return 0;
}

double system_monitor_calc_cpu_percent(void) {
    cpu_times_t curr_cpu_times;

    if (read_cpu_times(&curr_cpu_times) < 0) {
        return -1.0;
    }

    // Calculate total and idle time differences
    uint64_t prev_total = prev_cpu_times.user + prev_cpu_times.nice +
                         prev_cpu_times.system + prev_cpu_times.idle +
                         prev_cpu_times.iowait + prev_cpu_times.irq +
                         prev_cpu_times.softirq + prev_cpu_times.steal;

    uint64_t curr_total = curr_cpu_times.user + curr_cpu_times.nice +
                         curr_cpu_times.system + curr_cpu_times.idle +
                         curr_cpu_times.iowait + curr_cpu_times.irq +
                         curr_cpu_times.softirq + curr_cpu_times.steal;

    uint64_t prev_idle = prev_cpu_times.idle + prev_cpu_times.iowait;
    uint64_t curr_idle = curr_cpu_times.idle + curr_cpu_times.iowait;

    uint64_t total_diff = curr_total - prev_total;
    uint64_t idle_diff = curr_idle - prev_idle;

    double cpu_percent = 0.0;
    if (total_diff > 0) {
        cpu_percent = 100.0 * (total_diff - idle_diff) / total_diff;
    }

    // Update previous times
    prev_cpu_times = curr_cpu_times;

    return cpu_percent;
}

int system_monitor_get_memory(system_stats_t *stats) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        return -1;
    }

    char buffer[256];
    uint64_t mem_total = 0, mem_free = 0, mem_available = 0;
    uint64_t swap_total = 0, swap_free = 0, buffers = 0, cached = 0;
    int found_fields = 0;

    while (fgets(buffer, sizeof(buffer), fp)) {
        if (sscanf(buffer, "MemTotal: %lu kB", &mem_total) == 1) {
            found_fields++;
        } else if (sscanf(buffer, "MemFree: %lu kB", &mem_free) == 1) {
            found_fields++;
        } else if (sscanf(buffer, "MemAvailable: %lu kB", &mem_available) == 1) {
            found_fields++;
        } else if (sscanf(buffer, "Buffers: %lu kB", &buffers) == 1) {
            found_fields++;
        } else if (sscanf(buffer, "Cached: %lu kB", &cached) == 1) {
            found_fields++;
        } else if (sscanf(buffer, "SwapTotal: %lu kB", &swap_total) == 1) {
            found_fields++;
        } else if (sscanf(buffer, "SwapFree: %lu kB", &swap_free) == 1) {
            found_fields++;
        }

        if (found_fields >= 7) {
            break;
        }
    }

    fclose(fp);

    if (found_fields < 4) {
        return -1;
    }

    stats->total_mem_kb = mem_total;
    stats->free_mem_kb = mem_free;
    stats->used_mem_kb = mem_total - mem_available;
    stats->total_swap_kb = swap_total;
    stats->used_swap_kb = swap_total - swap_free;

    return 0;
}

int system_monitor_get_stats(system_stats_t *stats) {
    if (!initialized) {
        if (system_monitor_init() < 0) {
            return -1;
        }
    }

    stats->cpu_percent = system_monitor_calc_cpu_percent();

    if (system_monitor_get_memory(stats) < 0) {
        return -1;
    }

    return 0;
}

void system_monitor_cleanup(void) {
    initialized = 0;
}
