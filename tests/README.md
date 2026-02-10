# Enshrouded Monitor - Test Suite

Automated testing infrastructure for the Enshrouded Monitor project.

## Test Organization

### Unit Tests

#### `test_formatting.c`
Tests for formatting utility functions:
- `format_bytes()` - KB/MB/GB formatting
- `format_uptime()` - Human-readable uptime formatting

**Coverage:**
- Normal cases (KB, MB, GB)
- Edge cases (zero, very large values)
- Boundary conditions

#### `test_a2s_parsing.c`
Tests for A2S query parsing:
- `a2s_parse_server_status()` - Server status detection
- `a2s_status_string()` - Status string conversion

**Coverage:**
- Status detection (Lobby, Loading, Host Online)
- Case-insensitive matching
- String representations

#### `test_string_parsing.c`
**Security-focused tests** for buffer handling:
- `read_string()` - String extraction from binary buffers

**Coverage:**
- Buffer overflow scenarios
- Missing null terminators
- Truncation behavior
- Boundary conditions
- Malicious input handling

### Integration Tests

#### `../test_a2s`
Standalone A2S query test utility:
- Real UDP socket communication
- A2S_INFO protocol implementation
- Network timeout handling

## Running Tests

### Quick Start
```bash
# Run all unit tests
make unittest

# Run specific test
cd tests
make test_string_parsing
./test_string_parsing

# Run all tests (unit + integration)
make test-all
```

### Manual Testing
```bash
cd tests
make                    # Build all tests
make test              # Run all tests
make clean             # Clean build artifacts
```

### Individual Tests
```bash
./test_formatting       # Test formatting functions
./test_a2s_parsing     # Test A2S parsing logic
./test_string_parsing  # Test buffer security
```

## CI/CD Integration

Tests run automatically on:
- Every push to `main`, `develop`, or `feature/**` branches
- Every pull request
- GitHub Actions workflow: `.github/workflows/ci.yml`

### CI Pipeline Stages

1. **Build and Test**
   - Compile project
   - Run unit tests
   - Test utilities
   - Memory leak detection (valgrind)

2. **Static Analysis**
   - cppcheck for code quality
   - clang-tidy for best practices

3. **Security Scanning**
   - Security compiler flags
   - Banned function detection
   - Vulnerability patterns

## Test Framework

**Unity** - Lightweight C testing framework
- Single-header implementation (`unity.h`)
- No external dependencies
- Simple assertion macros
- Clear test output

### Writing New Tests

```c
#include "unity.h"

void test_my_function(void) {
    int result = my_function(5);
    TEST_ASSERT_EQUAL_INT(10, result);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_my_function);
    UNITY_END();
}
```

## Coverage Goals

- [x] Core formatting functions
- [x] A2S parsing logic
- [x] Buffer security
- [ ] System monitoring (/proc parsing)
- [ ] Process discovery
- [ ] Signal handling
- [ ] ncurses UI (manual testing)

## Memory Safety

All tests are compatible with:
- **Valgrind** - Memory leak detection
- **AddressSanitizer** - Buffer overflow detection
- **UndefinedBehaviorSanitizer** - UB detection

Run with sanitizers:
```bash
cd tests
CFLAGS="-fsanitize=address,undefined -g" make clean test
```

## Future Enhancements

1. **Mock Framework** - For isolating units
2. **Coverage Reports** - gcov/lcov integration
3. **Fuzzing** - AFL/libFuzzer for A2S parsing
4. **Performance Tests** - Benchmark suite
5. **Integration Tests** - Full system tests
