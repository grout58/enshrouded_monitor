#ifndef FORMATTING_H
#define FORMATTING_H

#include <stdint.h>
#include <stddef.h>

// Format bytes to human-readable format
void format_bytes(uint64_t kb, char *buffer, size_t buf_size);

// Format uptime to human-readable format
void format_uptime(uint64_t seconds, char *buffer, size_t buf_size);

#endif // FORMATTING_H
