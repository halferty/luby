# Luby Test Suite

This directory contains the test suite for the Luby interpreter.

## Running Tests

### Quick Start

Run all tests:
```bash
make test
```

Or use the test runner directly:
```bash
./run_tests.sh
```

### Individual Tests

Compile and run a specific test:
```bash
gcc -o tests/numeric tests/numeric.c -I. -std=c99 -lm
./tests/numeric
```

## Test Files

- **basic.c** - Basic functionality tests (variables, operators, control flow)
- **features.c** - Language features (unless, until, case/when, super, ranges, ternary, ivars)
- **missing.c** - Tests for missing features/edge cases
- **more.c** - Additional functionality tests
- **vfs.c** - Virtual filesystem tests
- **reflection.c** - Reflection and type checking (is_a?, kind_of?, instance_of?, defined?)
- **file_line.c** - `__FILE__` and `__LINE__` special variables
- **visibility.c** - Visibility modifiers (private, public, protected) and alias
- **numeric.c** - Numeric predicates (zero?, positive?, negative?, even?, odd?, abs, ceil, floor, round)

## Test Status

| Test | Status | Notes |
|------|--------|-------|
| basic | ✅ | All basic functionality tests |
| features | ✅ | All pass |
| missing | ✅ | All pass |
| more | ✅ | All pass |
| vfs | ✅ | All pass |
| reflection | ✅ | All pass |
| file_line | ✅ | All pass (9/9) |
| visibility | ✅ | All pass (10/10) |
| numeric | ✅ | All pass (35/35) |

## Writing New Tests

Tests follow a simple pattern:

```c
#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>

static int test(luby_state *L, const char *name, const char *code) {
    luby_value out;
    int rc = luby_eval(L, code, 0, "<test>", &out);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL %s: %s\n", name, buf);
        return 0;
    }
    printf("PASS %s\n", name);
    return 1;
}

int main(void) {
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    
    test(L, "test name", "ruby_code_here");
    
    luby_free(L);
    return 0;
}
```

## Cleaning Up

Remove all compiled test binaries:
```bash
make clean
```
