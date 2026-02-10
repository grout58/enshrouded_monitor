/*
 * Security-focused tests for P1 vulnerability fixes
 * Tests the hardened read_string() and A2S parsing
 */

#include "unity.h"
#include <stdint.h>
#include <string.h>

// Hardened version with security fixes
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

// ========== P1 Security Tests ==========

void test_null_buffer_pointer(void) {
    char dest[64];
    int offset = 0;

    int result = read_string(NULL, 100, dest, sizeof(dest), &offset);

    TEST_ASSERT_EQUAL_INT(-1, result);
    TEST_ASSERT_EQUAL_STRING("", dest); // Should set empty string
}

void test_null_dest_pointer(void) {
    uint8_t buffer[] = "Test\0";
    int offset = 0;

    int result = read_string(buffer, sizeof(buffer), NULL, 64, &offset);

    TEST_ASSERT_EQUAL_INT(-1, result);
    // No crash = success
}

void test_null_offset_pointer(void) {
    uint8_t buffer[] = "Test\0";
    char dest[64];

    int result = read_string(buffer, sizeof(buffer), dest, sizeof(dest), NULL);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_negative_offset(void) {
    uint8_t buffer[] = "Test\0";
    char dest[64];
    int offset = -5;

    int result = read_string(buffer, sizeof(buffer), dest, sizeof(dest), &offset);

    TEST_ASSERT_EQUAL_INT(-1, result);
    TEST_ASSERT_EQUAL_STRING("", dest);
}

void test_offset_beyond_buffer(void) {
    uint8_t buffer[] = "Test\0";
    char dest[64];
    int offset = 100; // Way beyond buffer

    int result = read_string(buffer, sizeof(buffer), dest, sizeof(dest), &offset);

    TEST_ASSERT_EQUAL_INT(-1, result);
    TEST_ASSERT_EQUAL_STRING("", dest);
}

void test_offset_at_exact_boundary(void) {
    uint8_t buffer[] = "Test\0";
    char dest[64];
    int offset = 5; // Exactly at end (past the string)

    int result = read_string(buffer, 5, dest, sizeof(dest), &offset);

    TEST_ASSERT_EQUAL_INT(-1, result); // Should fail, offset >= max_len
}

void test_malicious_packet_no_nulls(void) {
    // Simulate malicious A2S packet with no null terminators
    uint8_t buffer[100];
    memset(buffer, 'A', sizeof(buffer)); // Fill with 'A', no null bytes

    char dest[64];
    int offset = 0;

    int result = read_string(buffer, sizeof(buffer), dest, sizeof(dest), &offset);

    // Should read up to dest_size-1 and null-terminate
    TEST_ASSERT_EQUAL_INT(63, result); // Reads 63 chars (dest_size - 1)
    TEST_ASSERT_EQUAL_INT(63, strlen(dest));
    TEST_ASSERT_TRUE(dest[63] == '\0'); // Must be null-terminated
}

void test_malicious_packet_truncated(void) {
    // Packet that claims to have more data but is truncated
    uint8_t buffer[] = "ServerName\0Map\0"; // Only 2 strings
    char dest1[64], dest2[64], dest3[64];
    int offset = 0;

    // Read first string - OK
    int result1 = read_string(buffer, sizeof(buffer), dest1, sizeof(dest1), &offset);
    TEST_ASSERT_TRUE(result1 >= 0);

    // Read second string - OK
    int result2 = read_string(buffer, sizeof(buffer), dest2, sizeof(dest2), &offset);
    TEST_ASSERT_TRUE(result2 >= 0);

    // Try to read third string - should fail or return empty
    int result3 = read_string(buffer, sizeof(buffer), dest3, sizeof(dest3), &offset);
    // Should either fail or return empty string
    TEST_ASSERT_TRUE(result3 == -1 || strlen(dest3) == 0);
}

void test_zero_length_strings(void) {
    uint8_t buffer[] = "\0\0\0"; // Three empty strings
    char dest[64];
    int offset = 0;

    int result = read_string(buffer, sizeof(buffer), dest, sizeof(dest), &offset);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("", dest);
    TEST_ASSERT_EQUAL_INT(1, offset); // Should advance past null terminator
}

void test_max_length_string(void) {
    // String exactly at max_len boundary
    uint8_t buffer[256];
    memset(buffer, 'X', 255);
    buffer[255] = '\0';

    char dest[300];
    int offset = 0;

    int result = read_string(buffer, 256, dest, sizeof(dest), &offset);

    TEST_ASSERT_EQUAL_INT(255, result);
    TEST_ASSERT_EQUAL_INT(255, strlen(dest));
    TEST_ASSERT_EQUAL_INT(256, offset); // Should be at position 256
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_null_buffer_pointer);
    RUN_TEST(test_null_dest_pointer);
    RUN_TEST(test_null_offset_pointer);
    RUN_TEST(test_negative_offset);
    RUN_TEST(test_offset_beyond_buffer);
    RUN_TEST(test_offset_at_exact_boundary);
    RUN_TEST(test_malicious_packet_no_nulls);
    RUN_TEST(test_malicious_packet_truncated);
    RUN_TEST(test_zero_length_strings);
    RUN_TEST(test_max_length_string);

    UNITY_END();
}
