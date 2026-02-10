# Testing Infrastructure

Comprehensive automated testing for Enshrouded Monitor.

## ✅ Test Coverage

### Unit Tests (26 tests total)

#### Formatting Functions (9 tests)
- ✅ Byte formatting (KB/MB/GB)
- ✅ Large value handling (16GB+)
- ✅ Uptime formatting (seconds, minutes, hours, days)
- ✅ Edge cases (zero values)

#### A2S Query Parsing (10 tests)
- ✅ Server status detection
- ✅ Case-insensitive matching
- ✅ Lobby/Loading/Online states
- ✅ Status string conversion

#### Buffer Security (7 tests)
- ✅ Normal string parsing
- ✅ Multiple consecutive strings
- ✅ Missing null terminators
- ✅ Buffer truncation
- ✅ Empty strings
- ✅ Boundary conditions
- ✅ Out-of-bounds protection

### Integration Tests
- ✅ A2S query protocol (test_a2s utility)

## 🚀 Quick Start

```bash
# Run all tests
make unittest

# Run specific test suite
cd tests
./test_formatting       # Formatting tests
./test_a2s_parsing     # A2S parsing tests
./test_string_parsing  # Security tests

# Build and run all
cd tests && make test
```

## 📊 Test Results

All 26 unit tests passing ✅

```
test_formatting:       9/9 passed
test_a2s_parsing:     10/10 passed
test_string_parsing:   7/7 passed
```

## 🔧 CI/CD Pipeline

Automated testing on every push and PR via GitHub Actions:

### Build & Test
- ✅ Compile with strict warnings
- ✅ Run all unit tests
- ✅ Integration test utilities
- ✅ Memory leak detection (valgrind)

### Static Analysis
- ✅ cppcheck for code quality
- ✅ clang-tidy for best practices

### Security Scanning
- ✅ Security compiler flags (-D_FORTIFY_SOURCE=2, -fstack-protector-strong)
- ✅ Banned function detection (gets, sprintf)
- ✅ Build with -Werror

## 🛡️ Security Testing

The test suite specifically targets security vulnerabilities identified in code review:

### Buffer Overflow Protection
```c
// Tests verify safe handling of:
- Malformed A2S responses
- Missing null terminators
- Truncated packets
- Out-of-bounds offsets
```

### Memory Safety
- No memory leaks (valgrind clean)
- Proper bounds checking
- Safe string operations

## 📈 Future Enhancements

### Phase 3 Tests (Planned)
- [ ] inotify log monitoring
- [ ] Log parsing with regex
- [ ] Player tracking
- [ ] Connection duration calculation

### Advanced Testing
- [ ] Fuzzing for A2S parser (AFL/libFuzzer)
- [ ] Coverage reports (gcov/lcov)
- [ ] Performance benchmarks
- [ ] Mock framework for isolation
- [ ] System call mocking

## 🔬 Running with Sanitizers

### AddressSanitizer (buffer overflows)
```bash
cd tests
CFLAGS="-fsanitize=address -g" make clean test
```

### UndefinedBehaviorSanitizer
```bash
cd tests
CFLAGS="-fsanitize=undefined -g" make clean test
```

### Memory Leak Detection
```bash
cd tests
make clean && make
valgrind --leak-check=full ./test_string_parsing
```

## 📝 Writing New Tests

1. Create `tests/test_yourfeature.c`
2. Include `unity.h`
3. Write test functions
4. Add to `tests/Makefile`

Example:
```c
#include "unity.h"

void test_my_feature(void) {
    int result = my_function(42);
    TEST_ASSERT_EQUAL_INT(84, result);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_my_feature);
    UNITY_END();
}
```

## 🎯 Testing Philosophy

1. **Security First** - All parsing code is tested for buffer overflows
2. **Automated** - Tests run on every push
3. **Fast** - Unit tests complete in < 1 second
4. **Comprehensive** - Edge cases and boundary conditions
5. **Maintainable** - Clear test names and simple assertions

## 📚 Resources

- Test framework: Unity (lightweight C framework)
- CI/CD: GitHub Actions
- Static analysis: cppcheck, clang-tidy
- Memory testing: valgrind, AddressSanitizer

---

**Status:** All tests passing ✅
**Coverage:** Core functionality + security-critical paths
**CI:** Automated on GitHub
