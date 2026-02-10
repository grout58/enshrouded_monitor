/*
 * Standalone A2S Query Test Utility
 * Usage: ./test_a2s <host> [port]
 */

#include <stdio.h>
#include <stdlib.h>
#include "a2s_query.h"

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s <host> [port]\n", program_name);
    fprintf(stderr, "\nArguments:\n");
    fprintf(stderr, "  host     Server hostname or IP address (required)\n");
    fprintf(stderr, "  port     Query port (default: 15637)\n");
    fprintf(stderr, "\nExamples:\n");
    fprintf(stderr, "  %s 10.0.2.33\n", program_name);
    fprintf(stderr, "  %s 10.0.2.33 15637\n", program_name);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error: Server host is required\n\n");
        print_usage(argv[0]);
        return 1;
    }

    const char *host = argv[1];
    uint16_t port = 15637;

    if (argc >= 3) {
        port = (uint16_t)atoi(argv[2]);
        if (port == 0) {
            fprintf(stderr, "Error: Invalid port number '%s'\n", argv[2]);
            return 1;
        }
    }

    if (argc > 3) {
        fprintf(stderr, "Error: Too many arguments\n\n");
        print_usage(argv[0]);
        return 1;
    }

    printf("Testing A2S Query on %s:%d\n", host, port);
    printf("=====================================\n\n");

    // Initialize A2S query
    if (a2s_query_init(host, port) < 0) {
        fprintf(stderr, "Failed to initialize A2S query\n");
        return 1;
    }

    // Query server info
    a2s_info_t info;
    int result = a2s_query_info(&info);

    if (result == 0) {
        printf("✓ Query successful!\n\n");
        printf("Server Name:  %s\n", info.name);
        printf("Version:      %s\n", info.version);
        printf("Map:          %s\n", info.map);
        printf("Game:         %s\n", info.game);
        printf("Folder:       %s\n", info.folder);
        printf("Players:      %d/%d\n", info.players, info.max_players);
        printf("Bots:         %d\n", info.bots);
        printf("Server Type:  %c\n", info.server_type);
        printf("Environment:  %c\n", info.environment);
        printf("Visibility:   %s\n", info.visibility ? "Private" : "Public");
        printf("VAC:          %s\n", info.vac ? "Secured" : "Unsecured");
        printf("Protocol:     %d\n", info.protocol);
        printf("App ID:       %d\n", info.app_id);
        printf("\nParsed Status: %s\n", a2s_status_string(info.status));
    } else if (result == -2) {
        printf("✗ Query timeout - server not responding\n");
        printf("\nPossible reasons:\n");
        printf("  • Server is not running\n");
        printf("  • Query port (UDP %d) is not open\n", port);
        printf("  • Firewall blocking UDP traffic\n");
        printf("  • Server has query protocol disabled\n");
    } else {
        printf("✗ Query failed with error code %d\n", result);
    }

    // Cleanup
    a2s_query_cleanup();

    return (result == 0) ? 0 : 1;
}
