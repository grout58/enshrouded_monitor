/*
 * Formatting utility functions
 * Extracted for testing without ncurses dependency
 */

#include "formatting.h"
#include <stdio.h>

void format_bytes(uint64_t kb, char *buffer, size_t buf_size) {
    if (kb < 1024ULL) {
        snprintf(buffer, buf_size, "%lu KB", (unsigned long)kb);
    } else if (kb < 1024ULL * 1024ULL) {
        snprintf(buffer, buf_size, "%.1f MB", kb / 1024.0);
    } else {
        snprintf(buffer, buf_size, "%.2f GB", kb / (1024.0 * 1024.0));
    }
}

void format_uptime(uint64_t seconds, char *buffer, size_t buf_size) {
    uint64_t days = seconds / 86400;
    uint64_t hours = (seconds % 86400) / 3600;
    uint64_t mins = (seconds % 3600) / 60;
    uint64_t secs = seconds % 60;

    if (days > 0) {
        snprintf(buffer, buf_size, "%lud %02luh %02lum",
                (unsigned long)days, (unsigned long)hours, (unsigned long)mins);
    } else if (hours > 0) {
        snprintf(buffer, buf_size, "%luh %02lum %02lus",
                (unsigned long)hours, (unsigned long)mins, (unsigned long)secs);
    } else if (mins > 0) {
        snprintf(buffer, buf_size, "%lum %02lus", (unsigned long)mins, (unsigned long)secs);
    } else {
        snprintf(buffer, buf_size, "%lus", (unsigned long)secs);
    }
}
