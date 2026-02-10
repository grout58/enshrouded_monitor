#include "process_monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>

static long boot_time = 0;

// Get system boot time from /proc/stat
static long get_boot_time(void) {
    if (boot_time != 0) {
        return boot_time;
    }

    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        return -1;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (sscanf(buffer, "btime %ld", &boot_time) == 1) {
            break;
        }
    }

    fclose(fp);
    return boot_time;
}

// Check if a string is a valid PID (all digits)
static int is_pid(const char *str) {
    while (*str) {
        if (!isdigit(*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

// Read process name from /proc/[pid]/comm
static int read_process_name(pid_t pid, char *name, size_t max_len) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/comm", pid);

    FILE *fp = fopen(path, "r");
    if (!fp) {
        return -1;
    }

    if (!fgets(name, max_len, fp)) {
        fclose(fp);
        return -1;
    }

    fclose(fp);

    // Remove trailing newline
    size_t len = strlen(name);
    if (len > 0 && name[len - 1] == '\n') {
        name[len - 1] = '\0';
    }

    return 0;
}

// Read process cmdline from /proc/[pid]/cmdline (for Wine processes)
static int read_process_cmdline(pid_t pid, char *cmdline, size_t max_len) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);

    FILE *fp = fopen(path, "r");
    if (!fp) {
        return -1;
    }

    size_t read = fread(cmdline, 1, max_len - 1, fp);
    fclose(fp);

    if (read == 0) {
        return -1;
    }

    cmdline[read] = '\0';

    // Replace null bytes with spaces for readability
    for (size_t i = 0; i < read; i++) {
        if (cmdline[i] == '\0') {
            cmdline[i] = ' ';
        }
    }

    return 0;
}

int process_get_memory(pid_t pid, uint64_t *rss_kb) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    FILE *fp = fopen(path, "r");
    if (!fp) {
        return -1;
    }

    char buffer[256];
    int found = 0;

    while (fgets(buffer, sizeof(buffer), fp)) {
        if (sscanf(buffer, "VmRSS: %lu kB", rss_kb) == 1) {
            found = 1;
            break;
        }
    }

    fclose(fp);
    return found ? 0 : -1;
}

uint64_t process_get_uptime(pid_t pid) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    FILE *fp = fopen(path, "r");
    if (!fp) {
        return 0;
    }

    char buffer[1024];
    if (!fgets(buffer, sizeof(buffer), fp)) {
        fclose(fp);
        return 0;
    }
    fclose(fp);

    // Parse the 22nd field (starttime) from /proc/[pid]/stat
    // Format: pid (comm) state ppid ... starttime ...
    char *p = strchr(buffer, ')');
    if (!p) {
        return 0;
    }
    p += 2; // Skip ") "

    unsigned long long starttime = 0;
    int field = 3; // We're now at field 3 (state)

    while (field < 22 && *p) {
        while (*p && *p != ' ') p++;
        while (*p && *p == ' ') p++;
        field++;
    }

    if (field == 22) {
        sscanf(p, "%llu", &starttime);
    }

    long btime = get_boot_time();
    if (btime <= 0) {
        return 0;
    }

    // starttime is in clock ticks since boot
    long clock_ticks = sysconf(_SC_CLK_TCK);
    if (clock_ticks <= 0) {
        clock_ticks = 100; // Default fallback
    }

    time_t process_start = btime + (starttime / clock_ticks);
    time_t now = time(NULL);

    return (now > process_start) ? (now - process_start) : 0;
}

int process_find_by_name(const char *target_name, process_info_t *info) {
    DIR *proc_dir = opendir("/proc");
    if (!proc_dir) {
        return -1;
    }

    struct dirent *entry;
    int found = 0;

    while ((entry = readdir(proc_dir)) != NULL) {
        if (!is_pid(entry->d_name)) {
            continue;
        }

        pid_t pid = atoi(entry->d_name);
        char name[MAX_PROCESS_NAME];
        char cmdline[512];

        // Try comm first
        if (read_process_name(pid, name, sizeof(name)) == 0) {
            if (strcasestr(name, target_name) != NULL) {
                found = 1;
                info->pid = pid;
                strncpy(info->name, name, MAX_PROCESS_NAME - 1);
                info->name[MAX_PROCESS_NAME - 1] = '\0';
                break;
            }
        }

        // For Wine processes, check cmdline
        if (read_process_cmdline(pid, cmdline, sizeof(cmdline)) == 0) {
            if (strcasestr(cmdline, target_name) != NULL) {
                found = 1;
                info->pid = pid;
                strncpy(info->name, target_name, MAX_PROCESS_NAME - 1);
                info->name[MAX_PROCESS_NAME - 1] = '\0';
                break;
            }
        }
    }

    closedir(proc_dir);

    if (!found) {
        return -1;
    }

    // Get additional process info
    process_get_memory(info->pid, &info->rss_kb);
    info->uptime_seconds = process_get_uptime(info->pid);
    info->cpu_percent = 0.0; // TODO: Implement CPU percentage for specific process

    return 0;
}

int process_get_info(pid_t pid, process_info_t *info) {
    info->pid = pid;

    if (read_process_name(pid, info->name, MAX_PROCESS_NAME) < 0) {
        return -1;
    }

    if (process_get_memory(pid, &info->rss_kb) < 0) {
        info->rss_kb = 0;
    }

    info->uptime_seconds = process_get_uptime(pid);
    info->cpu_percent = 0.0;

    return 0;
}
