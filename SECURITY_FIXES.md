# Security Fixes - P1 Vulnerabilities

## Overview

This document details the P1 (Critical) security vulnerabilities identified in code review and their fixes.

## Vulnerabilities Fixed

### 🔴 P1-1: Buffer Overflow in `read_string()`

**Location:** `a2s_query.c:63-71` (original)

**Vulnerability:**
```c
// VULNERABLE CODE (original)
static int read_string(const uint8_t *buffer, int max_len, char *dest, int dest_size, int *offset) {
    int i = 0;
    while (*offset < max_len && buffer[*offset] != 0 && i < dest_size - 1) {
        dest[i++] = buffer[(*offset)++];
    }
    dest[i] = '\0';
    (*offset)++; // ❌ Increments without bounds checking
    return i;
}
```

**Attack Scenario:**
A malicious server could send a packet with strings that lack null terminators. When `*offset` reaches `max_len`, the loop exits, but the unconditional `(*offset)++` on line 69 would increment the offset beyond the buffer boundary, leading to out-of-bounds read on subsequent calls.

**Impact:**
- Buffer over-read (CWE-125)
- Potential information disclosure
- Undefined behavior
- Possible crash

**Fix:**
```c
// SECURE CODE (fixed)
static int read_string(const uint8_t *buffer, int max_len, char *dest, int dest_size, int *offset) {
    // ✅ Validate input parameters
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

    // ✅ Only skip null terminator if within bounds
    if (*offset < max_len && buffer[*offset] == 0) {
        (*offset)++;
    }

    return i;
}
```

**Security Improvements:**
1. Input validation (null pointers, negative offsets, out-of-bounds)
2. Bounds checking before incrementing offset
3. Error return code (-1) for invalid conditions
4. Defensive null-termination on error

---

### 🔴 P1-2: No Input Validation for A2S Response Parsing

**Location:** `a2s_query.c:190-230` (original)

**Vulnerability:**
```c
// VULNERABLE CODE (original)
int offset = 5;
info->protocol = buffer[offset++];

// ❌ No error checking on read_string() return values
read_string(buffer, received, info->name, MAX_SERVER_NAME, &offset);
read_string(buffer, received, info->map, MAX_MAP_NAME, &offset);
read_string(buffer, received, info->folder, MAX_GAME_NAME, &offset);
read_string(buffer, received, info->game, MAX_GAME_NAME, &offset);

// ❌ No validation that offset is still valid
if (offset + 2 <= received) {
    info->app_id = buffer[offset] | (buffer[offset + 1] << 8);
    offset += 2;
}
```

**Attack Scenario:**
A malicious server sends a truncated or malformed A2S_INFO packet. The code calls `read_string()` multiple times without checking return values. If the packet is truncated, `offset` could be in an invalid state, leading to reads from uninitialized memory or beyond the buffer.

**Impact:**
- Buffer over-read (CWE-125)
- Information disclosure
- Potential crash
- Acceptance of invalid server data

**Fix:**
```c
// SECURE CODE (fixed)
int offset = 5;

// ✅ Validate minimum packet size
if (received < 10) {
    return -1;
}

// ✅ Validate offset before each read
if (offset >= received) {
    return -1;
}
info->protocol = buffer[offset++];

// ✅ Check return values from read_string()
if (read_string(buffer, received, info->name, MAX_SERVER_NAME, &offset) < 0) {
    return -1;
}
if (read_string(buffer, received, info->map, MAX_MAP_NAME, &offset) < 0) {
    return -1;
}
if (read_string(buffer, received, info->folder, MAX_GAME_NAME, &offset) < 0) {
    return -1;
}
if (read_string(buffer, received, info->game, MAX_GAME_NAME, &offset) < 0) {
    return -1;
}

// ✅ Safe defaults for optional fields
if (offset + 2 <= received) {
    info->app_id = buffer[offset] | (buffer[offset + 1] << 8);
    offset += 2;
} else {
    info->app_id = 0; // Default value
}
```

**Security Improvements:**
1. Minimum packet size validation
2. Return value checking for all `read_string()` calls
3. Early return on parsing errors
4. Safe default values for optional fields
5. Explicit validation before every buffer access

---

### ✅ ALREADY FIXED: Integer Overflow in `format_bytes()`

**Status:** Fixed during refactoring

**Original Issue:**
```c
} else if (kb < 1024 * 1024) {  // ❌ Could overflow on 32-bit
```

**Fixed:**
```c
} else if (kb < 1024ULL * 1024ULL) {  // ✅ No overflow
```

---

## Test Coverage

### New Security Tests (10 tests)

**File:** `tests/test_security.c`

1. **test_null_buffer_pointer** - Validates null pointer handling
2. **test_null_dest_pointer** - Ensures no crash on null destination
3. **test_null_offset_pointer** - Validates null offset handling
4. **test_negative_offset** - Prevents negative offset exploitation
5. **test_offset_beyond_buffer** - Validates out-of-bounds detection
6. **test_offset_at_exact_boundary** - Edge case at buffer end
7. **test_malicious_packet_no_nulls** - Simulates attack packet without null terminators
8. **test_malicious_packet_truncated** - Validates truncated packet handling
9. **test_zero_length_strings** - Edge case testing
10. **test_max_length_string** - Boundary condition validation

**Results:** All 36 tests passing ✅

```
test_formatting:       9/9 passed
test_a2s_parsing:     10/10 passed
test_string_parsing:   7/7 passed
test_security:        10/10 passed
```

---

## Verification

### Functional Testing

✅ Application compiles without warnings
✅ A2S query successfully queries real server (10.0.2.33:15637)
✅ All parsing functions work correctly
✅ No regressions in functionality

### Security Testing

✅ Valgrind: No memory leaks
✅ AddressSanitizer: No buffer overflows
✅ All malicious input tests pass
✅ Fuzzing-ready (bounds checking in place)

---

## Attack Surface Reduction

**Before Fixes:**
- ❌ Accepts malformed packets silently
- ❌ No bounds checking on offset increments
- ❌ No input validation
- ❌ Potential for buffer over-read

**After Fixes:**
- ✅ Rejects malformed packets with error codes
- ✅ All offset operations bounds-checked
- ✅ Comprehensive input validation
- ✅ Safe defaults for missing data
- ✅ No undefined behavior

---

## Recommendations

### Deployment

1. **Replace old version** with this security-hardened version
2. **Monitor logs** for increased error rates (rejected malicious packets)
3. **Test with real servers** to ensure compatibility

### Future Enhancements

1. **Rate limiting** - Limit A2S queries per second
2. **Packet validation** - Additional protocol conformance checks
3. **Fuzzing** - Continuous fuzzing with AFL or libFuzzer
4. **Security audit** - External security review

---

## References

- **CWE-125**: Out-of-bounds Read
- **CWE-787**: Out-of-bounds Write
- **CWE-476**: NULL Pointer Dereference
- **OWASP**: Input Validation Failures

---

**Status:** All P1 vulnerabilities fixed and tested ✅
**Test Coverage:** 36/36 tests passing ✅
**Ready for Production:** Yes ✅
