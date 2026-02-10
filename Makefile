CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LDFLAGS = -lncurses -lm
TARGET = emon
SOURCES = main.c system_monitor.c process_monitor.c a2s_query.c formatting.c
HEADERS = system_monitor.h process_monitor.h a2s_query.h formatting.h
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean debug test unittest

all: $(TARGET)

test: test_a2s

test_a2s: test_a2s.c a2s_query.o
	$(CC) $(CFLAGS) test_a2s.c a2s_query.o -o test_a2s

# Run unit tests
unittest:
	@echo "Running unit tests..."
	@cd tests && $(MAKE) test

# Run all tests (integration + unit)
test-all: test unittest
	@echo "\n=== All Tests Complete ==="

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS += -g -DDEBUG
debug: clean $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET) test_a2s

run: $(TARGET)
	@echo "Usage: ./$(TARGET) <host> [port]"
	@echo "Example: ./$(TARGET) 10.0.2.33 15637"
