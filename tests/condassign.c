#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>

static int test_int(luby_state *L, const char *name, const char *code, int expected) {
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

static int test_nil(luby_state *L, const char *name, const char *code) {
    luby_value out;
    int rc = luby_eval(L, code, 0, "<test>", &out);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL %s: %s\n", name, buf);
        return 0;
    }
    if (out.type != LUBY_T_NIL) {
        printf("FAIL %s: expected nil, got ", name);
        luby_print_value(out);
        printf("\n");
        return 0;
    }
    printf("PASS %s\n", name);
    return 1;
}

static int test_bool(luby_state *L, const char *name, const char *code, int expected) {
    luby_value out;
    int rc = luby_eval(L, code, 0, "<test>", &out);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL %s: %s\n", name, buf);
        return 0;
    }
    if (out.type != LUBY_T_BOOL || out.as.b != expected) {
        printf("FAIL %s: expected %s, got ", name, expected ? "true" : "false");
        luby_print_value(out);
        printf("\n");
        return 0;
    }
    printf("PASS %s\n", name);
    return 1;
}

int main(void) {
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    int ok = 1;

    // === ||= ===
    ok &= test_int(L, "||= on nil",
        "x = nil; x ||= 42; x", 42);
    ok &= test_int(L, "||= on false",
        "x = false; x ||= 42; x", 42);
    ok &= test_int(L, "||= on truthy (no overwrite)",
        "x = 10; x ||= 42; x", 10);
    ok &= test_int(L, "||= on zero (truthy in Ruby)",
        "x = 0; x ||= 42; x", 0);

    // === &&= ===
    ok &= test_int(L, "&&= on truthy",
        "x = 10; x &&= 42; x", 42);
    ok &= test_nil(L, "&&= on nil",
        "x = nil; x &&= 42; x");
    ok &= test_bool(L, "&&= on false",
        "x = false; x &&= 42; x", 0);
    ok &= test_int(L, "&&= chain",
        "x = 1; x &&= 2; x &&= 3; x", 3);

    // === || value semantics ===
    ok &= test_int(L, "|| returns first truthy",
        "nil || 5", 5);
    ok &= test_int(L, "|| returns left if truthy",
        "3 || 5", 3);
    ok &= test_nil(L, "|| returns last if all falsy",
        "nil || false || nil");
    ok &= test_int(L, "|| chain finds truthy",
        "nil || false || 7", 7);

    // === && value semantics ===
    ok &= test_int(L, "&& returns right if both truthy",
        "3 && 5", 5);
    ok &= test_nil(L, "&& returns left if falsy",
        "nil && 5");
    ok &= test_bool(L, "&& returns false if false",
        "false && 5", 0);
    ok &= test_int(L, "&& chain all truthy",
        "1 && 2 && 3", 3);

    // === ||= with ivar ===
    ok &= test_int(L, "||= with ivar",
        "class C; def init; @x = nil; end; def set; @x ||= 99; @x; end; end; C.new.set()", 99);

    luby_free(L);
    printf("\ncondassign tests: %s\n", ok ? "ALL PASSED" : "SOME FAILED");
    return ok ? 0 : 1;
}
