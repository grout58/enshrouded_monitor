/*
 * Security tests for string parsing functions
 * Tests buffer overflow scenarios and boundary conditions
 */

#include "unity.h"
#include <stdint.h>
#include <string.h>

// Mock version of read_string for testing (will be replaced with actual)
static int read_string(const uint8_t *buffer, int max_len, char *dest, int dest_size, int *offset) {
    int i = 0;
    while (*offset < max_len && buffer[*offset] != 0 && i < dest_size - 1) {
        dest[i++] = buffer[(*offset)++];
    }
    dest[i] = '\0';

    // Only skip null terminator if we're still within bounds
    if (*offset < max_len && buffer[*offset] == 0) {
        (*offset)++;
    }
    return i;
}

void test_read_string_normal(void) {
    uint8_t buffer[] = "Hello\0World\0";
    char dest[64];
    int offset = 0;

    int len = read_string(buffer, sizeof(buffer), dest, sizeof(dest), &offset);

    TEST_ASSERT_EQUAL_STRING("Hello", dest);
    TEST_ASSERT_EQUAL_INT(5, len);
    TEST_ASSERT_EQUAL_INT(6, offset); // Points past null terminator
}

void test_read_string_multiple(void) {
    uint8_t buffer[] = "First\0Second\0Third\0";
    char dest[64];
    int offset = 0;

    read_string(buffer, sizeof(buffer), dest, sizeof(dest), &offset);
    TEST_ASSERT_EQUAL_STRING("First", dest);

    read_string(buffer, sizeof(buffer), dest, sizeof(dest), &offset);
    TEST_ASSERT_EQUAL_STRING("Second", dest);

    read_string(buffer, sizeof(buffer), dest, sizeof(dest), &offset);
    TEST_ASSERT_EQUAL_STRING("Third", dest);
}

void test_read_string_no_null_terminator(void) {
    // Create buffer without null terminator (string literals always have \0)
    uint8_t buffer[6] = {'N', 'o', 'N', 'u', 'l', 'l'};
    char dest[64];
    int offset = 0;

    int len = read_string(buffer, sizeof(buffer), dest, sizeof(dest), &offset);

    TEST_ASSERT_EQUAL_STRING("NoNull", dest);
    TEST_ASSERT_EQUAL_INT(6, len);
    TEST_ASSERT_EQUAL_INT(6, offset); // Should stop at buffer end
}

void test_read_string_truncation(void) {
    uint8_t buffer[] = "VeryLongStringThatShouldBeTruncated\0";
    char dest[10]; // Small buffer
    int offset = 0;

    read_string(buffer, sizeof(buffer), dest, sizeof(dest), &offset);

    TEST_ASSERT_EQUAL_INT(9, strlen(dest)); // Max 9 chars (dest_size - 1)
    TEST_ASSERT_NOT_NULL(strchr(dest, '\0')); // Should be null-terminated
}

void test_read_string_empty(void) {
    uint8_t buffer[] = "\0Next\0";
    char dest[64];
    int offset = 0;

    int len = read_string(buffer, sizeof(buffer), dest, sizeof(dest), &offset);

    TEST_ASSERT_EQUAL_STRING("", dest);
    TEST_ASSERT_EQUAL_INT(0, len);
    TEST_ASSERT_EQUAL_INT(1, offset);
}

void test_read_string_boundary(void) {
    uint8_t buffer[] = "Test";
    char dest[64];
    int offset = 0;

    // Read with exact buffer size (no null terminator)
    int len = read_string(buffer, 4, dest, sizeof(dest), &offset);

    TEST_ASSERT_EQUAL_STRING("Test", dest);
    TEST_ASSERT_EQUAL_INT(4, len);
    TEST_ASSERT_EQUAL_INT(4, offset); // Should not go past buffer
}

void test_read_string_offset_at_end(void) {
    uint8_t buffer[] = "Test\0";
    char dest[64];
    int offset = 5; // Start at end

    int len = read_string(buffer, 5, dest, sizeof(dest), &offset);

    TEST_ASSERT_EQUAL_STRING("", dest);
    TEST_ASSERT_EQUAL_INT(0, len);
    TEST_ASSERT_EQUAL_INT(5, offset); // Should not increment past end
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_read_string_normal);
    RUN_TEST(test_read_string_multiple);
    RUN_TEST(test_read_string_no_null_terminator);
    RUN_TEST(test_read_string_truncation);
    RUN_TEST(test_read_string_empty);
    RUN_TEST(test_read_string_boundary);
    RUN_TEST(test_read_string_offset_at_end);

    UNITY_END();
}
