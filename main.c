#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include "system_monitor.h"
#include "process_monitor.h"
#include "a2s_query.h"
#include "formatting.h"

#define REFRESH_INTERVAL_MS 1000
#define RAM_DANGER_THRESHOLD_GB 12
#define RAM_DANGER_THRESHOLD_KB (RAM_DANGER_THRESHOLD_GB * 1024 * 1024ULL)
#define DEFAULT_A2S_PORT 15637

static volatile int running = 1;

void signal_handler(int signum) {
    (void)signum;
    running = 0;
}

// Draw a progress bar
void draw_bar(int y, int x, const char *label, double percent, int width, int is_danger) {
    mvprintw(y, x, "%s", label);

    int bar_start = x + strlen(label);
    int filled = (int)(width * percent / 100.0);

    if (is_danger) {
        attron(COLOR_PAIR(2)); // Red for danger
    } else {
        attron(COLOR_PAIR(1)); // Green for normal
    }

    for (int i = 0; i < width; i++) {
        if (i < filled) {
            mvaddch(y, bar_start + i, '|');
        } else {
            mvaddch(y, bar_start + i, '-');
        }
    }

    if (is_danger) {
        attroff(COLOR_PAIR(2));
    } else {
        attroff(COLOR_PAIR(1));
    }

    mvprintw(y, bar_start + width + 1, "%.1f%%", percent);
}

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s <host> [port]\n", program_name);
    fprintf(stderr, "\nArguments:\n");
    fprintf(stderr, "  host     Server hostname or IP address (required)\n");
    fprintf(stderr, "  port     Query port (default: %d)\n", DEFAULT_A2S_PORT);
    fprintf(stderr, "\nExamples:\n");
    fprintf(stderr, "  %s 10.0.2.33\n", program_name);
    fprintf(stderr, "  %s 10.0.2.33 15637\n", program_name);
    fprintf(stderr, "  %s 192.168.1.100 25637\n", program_name);
}

int main(int argc, char *argv[]) {
    // Require host as positional argument
    if (argc < 2) {
        fprintf(stderr, "Error: Server host is required\n\n");
        print_usage(argv[0]);
        return 1;
    }

    const char *query_host = argv[1];
    uint16_t query_port = DEFAULT_A2S_PORT;

    // Optional port as second argument
    if (argc >= 3) {
        query_port = (uint16_t)atoi(argv[2]);
        if (query_port == 0) {
            fprintf(stderr, "Error: Invalid port number '%s'\n", argv[2]);
            return 1;
        }
    }

    if (argc > 3) {
        fprintf(stderr, "Error: Too many arguments\n\n");
        print_usage(argv[0]);
        return 1;
    }

    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Initialize system monitoring
    if (system_monitor_init() < 0) {
        fprintf(stderr, "Failed to initialize system monitoring\n");
        return 1;
    }

    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    timeout(REFRESH_INTERVAL_MS);

    // Enable colors
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        init_pair(2, COLOR_RED, COLOR_BLACK);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
        init_pair(4, COLOR_CYAN, COLOR_BLACK);
    }

    // Initialize A2S query with configured host/port
    int a2s_available = (a2s_query_init(query_host, query_port) == 0);

    int ch;
    system_stats_t stats;
    process_info_t server_process;
    a2s_info_t server_info;
    int server_found = 0;
    int a2s_query_success = 0;
    int query_counter = 0;

    while (running && (ch = getch()) != 'q') {
        query_counter++;
        clear();

        // Get system stats
        if (system_monitor_get_stats(&stats) < 0) {
            mvprintw(0, 0, "Error reading system stats");
            refresh();
            continue;
        }

        // Draw header
        attron(A_BOLD | COLOR_PAIR(4));
        mvprintw(0, 0, "=== Enshrouded Monitor (EMon) ===");
        attroff(A_BOLD | COLOR_PAIR(4));
        mvprintw(0, 60, "Press 'q' to quit");

        // Show query target
        attron(COLOR_PAIR(4));
        mvprintw(1, 0, "Query Target: %s:%d", query_host, query_port);
        attroff(COLOR_PAIR(4));

        // Draw CPU bar
        draw_bar(2, 0, "CPU:  ", stats.cpu_percent, 40, 0);

        // Draw RAM bar
        double ram_percent = 100.0 * stats.used_mem_kb / stats.total_mem_kb;
        int ram_danger = (stats.used_mem_kb > RAM_DANGER_THRESHOLD_KB);
        draw_bar(3, 0, "RAM:  ", ram_percent, 40, ram_danger);

        // RAM details
        char used_str[32], total_str[32];
        format_bytes(stats.used_mem_kb, used_str, sizeof(used_str));
        format_bytes(stats.total_mem_kb, total_str, sizeof(total_str));
        mvprintw(4, 6, "%s / %s", used_str, total_str);

        if (ram_danger) {
            attron(COLOR_PAIR(2) | A_BOLD);
            mvprintw(4, 30, "[DANGER: >%dGB]", RAM_DANGER_THRESHOLD_GB);
            attroff(COLOR_PAIR(2) | A_BOLD);
        }

        // Separator
        mvprintw(6, 0, "================================");

        // Search for Enshrouded server process
        server_found = (process_find_by_name("EnshroudedServer", &server_process) == 0);

        // Query A2S_INFO (every refresh if available)
        if (a2s_available && server_found) {
            int query_result = a2s_query_info(&server_info);
            if (query_result == 0) {
                a2s_query_success = 1;
            } else if (query_result == -2) {
                // Timeout - server not responding to queries
                a2s_query_success = 0;
            } else {
                a2s_query_success = 0;
            }
        }

        if (server_found) {
            // Status indicator based on A2S query
            if (a2s_query_success) {
                int color = COLOR_PAIR(1); // Green
                if (server_info.status == SERVER_STATUS_LOADING) {
                    color = COLOR_PAIR(3); // Yellow
                } else if (server_info.status == SERVER_STATUS_LOBBY) {
                    color = COLOR_PAIR(4); // Cyan
                }

                attron(A_BOLD | color);
                mvprintw(8, 0, "Server Status: %s", a2s_status_string(server_info.status));
                attroff(A_BOLD | color);
            } else {
                attron(A_BOLD | COLOR_PAIR(3));
                mvprintw(8, 0, "Server Status: RUNNING (Query Unavailable)");
                attroff(A_BOLD | COLOR_PAIR(3));
            }

            // Process info
            mvprintw(10, 0, "Process: %s", server_process.name);
            mvprintw(11, 0, "PID:     %d", server_process.pid);

            char uptime_str[64];
            format_uptime(server_process.uptime_seconds, uptime_str, sizeof(uptime_str));
            mvprintw(12, 0, "Uptime:  %s", uptime_str);

            char mem_str[32];
            format_bytes(server_process.rss_kb, mem_str, sizeof(mem_str));
            mvprintw(13, 0, "Memory:  %s", mem_str);

            // Separator
            mvprintw(15, 0, "--- Server Details (A2S Query) ---");

            // Server details from A2S query
            if (a2s_query_success) {
                mvprintw(16, 0, "Server Name: %s", server_info.name);
                mvprintw(17, 0, "Version:     %s", server_info.version);
                mvprintw(18, 0, "Players:     %d/%d", server_info.players, server_info.max_players);
                mvprintw(19, 0, "Map:         %s", server_info.map);
                mvprintw(20, 0, "Game:        %s", server_info.game);
            } else {
                attron(COLOR_PAIR(3));
                mvprintw(16, 0, "A2S Query: No response from %s:%d", query_host, query_port);
                mvprintw(17, 0, "Server may not have query port enabled or firewall blocking.");
                attroff(COLOR_PAIR(3));
            }

        } else {
            attron(A_BOLD | COLOR_PAIR(2));
            mvprintw(8, 0, "Server Status: NOT FOUND");
            attroff(A_BOLD | COLOR_PAIR(2));

            mvprintw(10, 0, "Searching for 'EnshroudedServer.exe' process...");
            mvprintw(11, 0, "Make sure the server is running via Wine/Proton.");
        }

        // Footer
        mvprintw(LINES - 2, 0, "================================");
        mvprintw(LINES - 1, 0, "Phase 2: A2S Query Integration | Query: %s",
                 a2s_available ? "Enabled" : "Unavailable");

        refresh();
    }

    // Cleanup
    endwin();
    system_monitor_cleanup();
    a2s_query_cleanup();

    return 0;
}
