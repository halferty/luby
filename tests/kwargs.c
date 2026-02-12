#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>

static int test(luby_state *L, const char *name, const char *code, int expected) {
    luby_value out;
    int rc = luby_eval(L, code, 0, "<test>", &out);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL %s: %s\n", name, buf);
        return 0;
    }
    if (out.type != LUBY_T_INT || out.as.i != expected) {
        printf("FAIL %s: expected %d, got ", name, expected);
        luby_print_value(out);
        printf("\n");
        return 0;
    }
    printf("PASS %s\n", name);
    return 1;
}

static int test_error(luby_state *L, const char *name, const char *code) {
    luby_value out;
    int rc = luby_eval(L, code, 0, "<test>", &out);
    if (rc != 0) {
        printf("PASS %s (got expected error)\n", name);
        return 1;
    }
    printf("FAIL %s: expected error but got success\n", name);
    return 0;
}

int main(void) {
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    int ok = 1;

    // Required keyword arg
    ok &= test(L, "required kwarg",
        "def f(x:); x; end; f(x: 42)", 42);

    // Default keyword arg
    ok &= test(L, "default kwarg",
        "def f(x: 10); x; end; f()", 10);

    // Default overridden
    ok &= test(L, "default kwarg overridden",
        "def f(x: 10); x; end; f(x: 5)", 5);

    // Multiple kwargs
    ok &= test(L, "multiple kwargs",
        "def f(a:, b:); a + b; end; f(a: 1, b: 2)", 3);

    // Mixed positional + kwargs
    ok &= test(L, "mixed positional and kwargs",
        "def f(x, y:); x + y; end; f(1, y: 2)", 3);

    // Order independence
    ok &= test(L, "kwarg order independence",
        "def f(a:, b:); a - b; end; f(b: 1, a: 10)", 9);

    // Missing required kwarg raises error
    ok &= test_error(L, "missing required kwarg",
        "def f(x:); x; end; f()");

    // Kwargs with default positional params
    ok &= test(L, "kwargs with default positional",
        "def f(x, y = 5, z:); x + y + z; end; f(1, z: 10)", 16);

    // Multiple kwargs with defaults
    ok &= test(L, "multiple kwargs with defaults",
        "def f(a: 1, b: 2); a + b; end; f()", 3);

    // Override one default kwarg
    ok &= test(L, "override one default kwarg",
        "def f(a: 1, b: 2); a * b; end; f(a: 10)", 20);

    // Kwargs in class methods
    ok &= test(L, "kwargs in class method",
        "class C; def add(x:, y:); x + y; end; end; C.new.add(x: 3, y: 7)", 10);

    luby_free(L);
    printf("\nkwargs tests: %s\n", ok ? "ALL PASSED" : "SOME FAILED");
    return ok ? 0 : 1;
}
