/* Unity - A lightweight C testing framework
 * Minimal single-header implementation
 * Based on Unity by ThrowTheSwitch.org
 */

#ifndef UNITY_H
#define UNITY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Test statistics
static int unity_tests_run = 0;
static int unity_tests_failed = 0;
static const char* unity_current_test = NULL;

// Macros
#define TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf("  FAIL: %s:%d - %s\n", __FILE__, __LINE__, #condition); \
            unity_tests_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL_INT(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf("  FAIL: %s:%d - Expected %d, got %d\n", __FILE__, __LINE__, (int)(expected), (int)(actual)); \
            unity_tests_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL_STRING(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            printf("  FAIL: %s:%d - Expected \"%s\", got \"%s\"\n", __FILE__, __LINE__, (expected), (actual)); \
            unity_tests_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_NOT_NULL(pointer) \
    do { \
        if ((pointer) == NULL) { \
            printf("  FAIL: %s:%d - Pointer is NULL\n", __FILE__, __LINE__); \
            unity_tests_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_NULL(pointer) \
    do { \
        if ((pointer) != NULL) { \
            printf("  FAIL: %s:%d - Pointer is not NULL\n", __FILE__, __LINE__); \
            unity_tests_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_TRUE(condition) TEST_ASSERT(condition)
#define TEST_ASSERT_FALSE(condition) TEST_ASSERT(!(condition))

#define RUN_TEST(test_func) \
    do { \
        unity_current_test = #test_func; \
        unity_tests_run++; \
        int prev_failed = unity_tests_failed; \
        test_func(); \
        if (unity_tests_failed == prev_failed) { \
            printf("  PASS: %s\n", #test_func); \
        } \
    } while(0)

// Test setup/teardown (optional)
void setUp(void) {}
void tearDown(void) {}

// Test runner
#define UNITY_BEGIN() \
    do { \
        printf("\n=== Running Tests ===\n"); \
        unity_tests_run = 0; \
        unity_tests_failed = 0; \
    } while(0)

#define UNITY_END() \
    do { \
        printf("\n=== Test Summary ===\n"); \
        printf("Tests run: %d\n", unity_tests_run); \
        printf("Tests passed: %d\n", unity_tests_run - unity_tests_failed); \
        printf("Tests failed: %d\n", unity_tests_failed); \
        return (unity_tests_failed == 0) ? 0 : 1; \
    } while(0)

#endif // UNITY_H
