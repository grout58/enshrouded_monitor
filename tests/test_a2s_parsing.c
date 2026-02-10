/*
 * Unit tests for A2S query parsing functions
 */

#include "unity.h"
#include <stdint.h>
#include <string.h>

// Forward declarations
typedef enum {
    SERVER_STATUS_UNKNOWN,
    SERVER_STATUS_LOBBY,
    SERVER_STATUS_LOADING,
    SERVER_STATUS_HOST_ONLINE
} server_status_t;

server_status_t a2s_parse_server_status(const char *server_name, const char *map_name);
const char* a2s_status_string(server_status_t status);

void test_status_lobby_name(void) {
    server_status_t status = a2s_parse_server_status("My Lobby Server", "");
    TEST_ASSERT_EQUAL_INT(SERVER_STATUS_LOBBY, status);
}

void test_status_lobby_map(void) {
    server_status_t status = a2s_parse_server_status("", "lobby_map");
    TEST_ASSERT_EQUAL_INT(SERVER_STATUS_LOBBY, status);
}

void test_status_loading_name(void) {
    server_status_t status = a2s_parse_server_status("Loading...", "");
    TEST_ASSERT_EQUAL_INT(SERVER_STATUS_LOADING, status);
}

void test_status_loading_map(void) {
    server_status_t status = a2s_parse_server_status("", "loading_screen");
    TEST_ASSERT_EQUAL_INT(SERVER_STATUS_LOADING, status);
}

void test_status_host_online(void) {
    server_status_t status = a2s_parse_server_status("Guntshrouded", "main_map");
    TEST_ASSERT_EQUAL_INT(SERVER_STATUS_HOST_ONLINE, status);
}

void test_status_case_insensitive(void) {
    server_status_t status = a2s_parse_server_status("LOBBY SERVER", "");
    TEST_ASSERT_EQUAL_INT(SERVER_STATUS_LOBBY, status);
}

void test_status_string_lobby(void) {
    const char *str = a2s_status_string(SERVER_STATUS_LOBBY);
    TEST_ASSERT_EQUAL_STRING("Lobby", str);
}

void test_status_string_loading(void) {
    const char *str = a2s_status_string(SERVER_STATUS_LOADING);
    TEST_ASSERT_EQUAL_STRING("Loading", str);
}

void test_status_string_online(void) {
    const char *str = a2s_status_string(SERVER_STATUS_HOST_ONLINE);
    TEST_ASSERT_EQUAL_STRING("Host Online", str);
}

void test_status_string_unknown(void) {
    const char *str = a2s_status_string(SERVER_STATUS_UNKNOWN);
    TEST_ASSERT_EQUAL_STRING("Unknown", str);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_status_lobby_name);
    RUN_TEST(test_status_lobby_map);
    RUN_TEST(test_status_loading_name);
    RUN_TEST(test_status_loading_map);
    RUN_TEST(test_status_host_online);
    RUN_TEST(test_status_case_insensitive);

    RUN_TEST(test_status_string_lobby);
    RUN_TEST(test_status_string_loading);
    RUN_TEST(test_status_string_online);
    RUN_TEST(test_status_string_unknown);

    UNITY_END();
}
