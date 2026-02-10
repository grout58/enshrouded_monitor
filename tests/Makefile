CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11 -I. -I..
LDFLAGS = -lm

# Source files
SRC_DIR = ..
SOURCES = $(SRC_DIR)/a2s_query.c

# Test files
TEST_SOURCES = test_formatting.c test_a2s_parsing.c test_string_parsing.c test_security.c
TEST_BINS = $(TEST_SOURCES:.c=)

# Utility sources that need to be compiled for tests
TEST_UTILS = test_utils.o

.PHONY: all clean run test

all: $(TEST_BINS)

# Build formatting tests (uses formatting.c)
test_formatting: test_formatting.c
	$(CC) $(CFLAGS) test_formatting.c $(SRC_DIR)/formatting.c -o test_formatting $(LDFLAGS)

# Build A2S parsing tests
test_a2s_parsing: test_a2s_parsing.c
	$(CC) $(CFLAGS) test_a2s_parsing.c $(SRC_DIR)/a2s_query.c -o test_a2s_parsing $(LDFLAGS)

# Build string parsing tests (standalone)
test_string_parsing: test_string_parsing.c
	$(CC) $(CFLAGS) test_string_parsing.c -o test_string_parsing $(LDFLAGS)

# Build security tests (P1 vulnerability fixes)
test_security: test_security.c
	$(CC) $(CFLAGS) test_security.c -o test_security $(LDFLAGS)

# Run all tests
test: all
	@echo "\n=== Running All Tests ==="
	@for test in $(TEST_BINS); do \
		echo "\n--- Running $$test ---"; \
		./$$test || exit 1; \
	done
	@echo "\n=== All Tests Passed ==="

run: test

clean:
	rm -f $(TEST_BINS) *.o main_test.o
