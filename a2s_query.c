#include "a2s_query.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>

static int sockfd = -1;
static struct sockaddr_in server_addr;

// A2S_INFO request packet
static const uint8_t a2s_info_request[] = {
    0xFF, 0xFF, 0xFF, 0xFF,  // Header
    0x54,                     // A2S_INFO
    'S', 'o', 'u', 'r', 'c', 'e', ' ',
    'E', 'n', 'g', 'i', 'n', 'e', ' ',
    'Q', 'u', 'e', 'r', 'y', 0x00
};

int a2s_query_init(const char *host, uint16_t port) {
    if (sockfd >= 0) {
        return 0; // Already initialized
    }

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    // Set socket timeout (2 seconds)
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        close(sockfd);
        sockfd = -1;
        return -1;
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        sockfd = -1;
        return -1;
    }

    return 0;
}

// Read a null-terminated string from buffer
// Returns -1 on error (offset out of bounds), or number of characters read
static int read_string(const uint8_t *buffer, int max_len, char *dest, int dest_size, int *offset) {
    // Validate input parameters
    if (!buffer || !dest || !offset || *offset < 0 || *offset >= max_len) {
        if (dest && dest_size > 0) {
            dest[0] = '\0';
        }
        return -1;
    }

    int i = 0;
    while (*offset < max_len && buffer[*offset] != 0 && i < dest_size - 1) {
        dest[i++] = buffer[(*offset)++];
    }
    dest[i] = '\0';

    // Only skip null terminator if we're still within bounds and found one
    if (*offset < max_len && buffer[*offset] == 0) {
        (*offset)++;
    }

    return i;
}

server_status_t a2s_parse_server_status(const char *server_name, const char *map_name) {
    // Parse server status from name or map
    // Common patterns:
    // - "Lobby" or contains "lobby" -> LOBBY
    // - "Loading" or contains "loading" -> LOADING
    // - Otherwise -> HOST_ONLINE

    char name_lower[MAX_SERVER_NAME];
    char map_lower[MAX_MAP_NAME];

    // Convert to lowercase for case-insensitive matching
    for (int i = 0; server_name[i] && i < MAX_SERVER_NAME - 1; i++) {
        name_lower[i] = tolower(server_name[i]);
    }
    name_lower[MAX_SERVER_NAME - 1] = '\0';

    for (int i = 0; map_name[i] && i < MAX_MAP_NAME - 1; i++) {
        map_lower[i] = tolower(map_name[i]);
    }
    map_lower[MAX_MAP_NAME - 1] = '\0';

    if (strstr(name_lower, "lobby") || strstr(map_lower, "lobby")) {
        return SERVER_STATUS_LOBBY;
    }

    if (strstr(name_lower, "loading") || strstr(map_lower, "loading")) {
        return SERVER_STATUS_LOADING;
    }

    // If server has players or is named, assume it's online
    return SERVER_STATUS_HOST_ONLINE;
}

const char* a2s_status_string(server_status_t status) {
    switch (status) {
        case SERVER_STATUS_LOBBY:
            return "Lobby";
        case SERVER_STATUS_LOADING:
            return "Loading";
        case SERVER_STATUS_HOST_ONLINE:
            return "Host Online";
        default:
            return "Unknown";
    }
}

int a2s_query_info(a2s_info_t *info) {
    if (sockfd < 0) {
        return -1;
    }

    // Send A2S_INFO request
    ssize_t sent = sendto(sockfd, a2s_info_request, sizeof(a2s_info_request), 0,
                         (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (sent < 0) {
        return -1;
    }

    // Receive response
    uint8_t buffer[4096];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    ssize_t received = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                                (struct sockaddr*)&from_addr, &from_len);
    if (received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Timeout - server not responding
            return -2;
        }
        return -1;
    }

    // Verify response header (0xFF 0xFF 0xFF 0xFF)
    if (received < 5 || buffer[0] != 0xFF || buffer[1] != 0xFF ||
        buffer[2] != 0xFF || buffer[3] != 0xFF) {
        return -1;
    }

    uint8_t response_type = buffer[4];

    // Handle challenge response (some servers require this)
    if (response_type == A2S_CHALLENGE_RESPONSE) {
        if (received < 9) {
            return -1;
        }

        // Extract challenge number (bytes 5-8)
        uint32_t challenge;
        memcpy(&challenge, &buffer[5], 4);

        // Resend request with challenge
        uint8_t challenge_request[sizeof(a2s_info_request) + 4];
        memcpy(challenge_request, a2s_info_request, sizeof(a2s_info_request));
        memcpy(&challenge_request[sizeof(a2s_info_request)], &challenge, 4);

        sent = sendto(sockfd, challenge_request, sizeof(challenge_request), 0,
                     (struct sockaddr*)&server_addr, sizeof(server_addr));
        if (sent < 0) {
            return -1;
        }

        // Receive actual response
        received = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                           (struct sockaddr*)&from_addr, &from_len);
        if (received < 0) {
            return -1;
        }

        response_type = buffer[4];
    }

    // Parse A2S_INFO response
    if (response_type != A2S_INFO_RESPONSE) {
        return -1;
    }

    int offset = 5; // Skip header and response type

    // Validate minimum packet size
    if (received < 10) {
        return -1;
    }

    // Parse protocol version
    if (offset >= received) {
        return -1;
    }
    info->protocol = buffer[offset++];

    // Parse strings with error checking
    if (read_string(buffer, received, info->name, MAX_SERVER_NAME, &offset) < 0) {
        return -1;
    }
    if (read_string(buffer, received, info->map, MAX_MAP_NAME, &offset) < 0) {
        return -1;
    }
    if (read_string(buffer, received, info->folder, MAX_GAME_NAME, &offset) < 0) {
        return -1;
    }
    if (read_string(buffer, received, info->game, MAX_GAME_NAME, &offset) < 0) {
        return -1;
    }

    // Parse app ID (2 bytes, little-endian)
    if (offset + 2 <= received) {
        info->app_id = buffer[offset] | (buffer[offset + 1] << 8);
        offset += 2;
    } else {
        info->app_id = 0;
    }

    // Parse player counts
    if (offset + 2 <= received) {
        info->players = buffer[offset++];
        info->max_players = buffer[offset++];
    } else {
        info->players = 0;
        info->max_players = 0;
    }

    // Parse bots
    if (offset + 1 <= received) {
        info->bots = buffer[offset++];
    } else {
        info->bots = 0;
    }

    // Parse server type ('d' = dedicated, 'l' = non-dedicated, 'p' = SourceTV)
    if (offset + 1 <= received) {
        info->server_type = buffer[offset++];
    } else {
        info->server_type = 'u'; // unknown
    }

    // Parse environment ('l' = Linux, 'w' = Windows, 'm' = Mac)
    if (offset + 1 <= received) {
        info->environment = buffer[offset++];
    } else {
        info->environment = 'u'; // unknown
    }

    // Parse visibility (0 = public, 1 = private)
    if (offset + 1 <= received) {
        info->visibility = buffer[offset++];
    } else {
        info->visibility = 0;
    }

    // Parse VAC (0 = unsecured, 1 = secured)
    if (offset + 1 <= received) {
        info->vac = buffer[offset++];
    } else {
        info->vac = 0;
    }

    // Parse version string with error checking
    if (read_string(buffer, received, info->version, MAX_VERSION_STRING, &offset) < 0) {
        // If version string is missing, use empty string
        info->version[0] = '\0';
    }

    // Determine server status
    info->status = a2s_parse_server_status(info->name, info->map);

    return 0;
}

void a2s_query_cleanup(void) {
    if (sockfd >= 0) {
        close(sockfd);
        sockfd = -1;
    }
}
