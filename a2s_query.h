#ifndef A2S_QUERY_H
#define A2S_QUERY_H

#include <stdint.h>
#include <netinet/in.h>

#define A2S_INFO_REQUEST 0x54
#define A2S_INFO_RESPONSE 0x49
#define A2S_CHALLENGE_RESPONSE 0x41

#define MAX_SERVER_NAME 256
#define MAX_MAP_NAME 128
#define MAX_GAME_NAME 64
#define MAX_VERSION_STRING 32

typedef enum {
    SERVER_STATUS_UNKNOWN,
    SERVER_STATUS_LOBBY,
    SERVER_STATUS_LOADING,
    SERVER_STATUS_HOST_ONLINE
} server_status_t;

typedef struct {
    uint8_t protocol;
    char name[MAX_SERVER_NAME];
    char map[MAX_MAP_NAME];
    char folder[MAX_GAME_NAME];
    char game[MAX_GAME_NAME];
    uint16_t app_id;
    uint8_t players;
    uint8_t max_players;
    uint8_t bots;
    char server_type;
    char environment;
    uint8_t visibility;
    uint8_t vac;
    char version[MAX_VERSION_STRING];
    server_status_t status;
} a2s_info_t;

// Initialize A2S query system
int a2s_query_init(const char *host, uint16_t port);

// Query server info using A2S_INFO protocol
int a2s_query_info(a2s_info_t *info);

// Determine server status from server name or map
server_status_t a2s_parse_server_status(const char *server_name, const char *map_name);

// Get status string
const char* a2s_status_string(server_status_t status);

// Cleanup A2S query system
void a2s_query_cleanup(void);

#endif // A2S_QUERY_H
