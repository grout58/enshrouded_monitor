#ifndef PROCESS_MONITOR_H
#define PROCESS_MONITOR_H

#include <stdint.h>
#include <time.h>

#define MAX_PROCESS_NAME 256

typedef struct {
    pid_t pid;
    char name[MAX_PROCESS_NAME];
    uint64_t rss_kb;          // Resident Set Size in KB
    double cpu_percent;
    time_t start_time;        // Process start time
    uint64_t uptime_seconds;  // Process uptime
} process_info_t;

// Find process by name (e.g., "EnshroudedServer.exe")
int process_find_by_name(const char *name, process_info_t *info);

// Get detailed process information by PID
int process_get_info(pid_t pid, process_info_t *info);

// Calculate process uptime
uint64_t process_get_uptime(pid_t pid);

// Get process memory usage
int process_get_memory(pid_t pid, uint64_t *rss_kb);

#endif // PROCESS_MONITOR_H
