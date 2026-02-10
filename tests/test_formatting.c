/*
 * Unit tests for formatting functions
 */

#include "unity.h"
#include "../formatting.h"
#include <stdint.h>
#include <string.h>

void test_format_bytes_kb(void) {
    char buffer[64];
    format_bytes(512, buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL_STRING("512 KB", buffer);
}

void test_format_bytes_mb(void) {
    char buffer[64];
    format_bytes(1536, buffer, sizeof(buffer)); // 1.5 MB
    TEST_ASSERT_EQUAL_STRING("1.5 MB", buffer);
}

void test_format_bytes_gb(void) {
    char buffer[64];
    format_bytes(2097152, buffer, sizeof(buffer)); // 2 GB
    TEST_ASSERT_EQUAL_STRING("2.00 GB", buffer);
}

void test_format_bytes_large(void) {
    char buffer[64];
    format_bytes(16777216ULL, buffer, sizeof(buffer)); // 16 GB
    TEST_ASSERT_EQUAL_STRING("16.00 GB", buffer);
}

void test_format_uptime_seconds(void) {
    char buffer[64];
    format_uptime(45, buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL_STRING("45s", buffer);
}

void test_format_uptime_minutes(void) {
    char buffer[64];
    format_uptime(125, buffer, sizeof(buffer)); // 2m 5s
    TEST_ASSERT_EQUAL_STRING("2m 05s", buffer);
}

void test_format_uptime_hours(void) {
    char buffer[64];
    format_uptime(7265, buffer, sizeof(buffer)); // 2h 1m 5s
    TEST_ASSERT_EQUAL_STRING("2h 01m 05s", buffer);
}

void test_format_uptime_days(void) {
    char buffer[64];
    format_uptime(90125, buffer, sizeof(buffer)); // 1d 1h 2m
    TEST_ASSERT_EQUAL_STRING("1d 01h 02m", buffer);
}

void test_format_uptime_zero(void) {
    char buffer[64];
    format_uptime(0, buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL_STRING("0s", buffer);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_format_bytes_kb);
    RUN_TEST(test_format_bytes_mb);
    RUN_TEST(test_format_bytes_gb);
    RUN_TEST(test_format_bytes_large);

    RUN_TEST(test_format_uptime_seconds);
    RUN_TEST(test_format_uptime_minutes);
    RUN_TEST(test_format_uptime_hours);
    RUN_TEST(test_format_uptime_days);
    RUN_TEST(test_format_uptime_zero);

    UNITY_END();
}
