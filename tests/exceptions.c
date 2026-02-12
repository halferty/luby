#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>
#include <string.h>

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

static int test_str(luby_state *L, const char *name, const char *code, const char *expected) {
    luby_value out;
    int rc = luby_eval(L, code, 0, "<test>", &out);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL %s: %s\n", name, buf);
        return 0;
    }
    if (out.type != LUBY_T_STRING || !out.as.ptr || strcmp((const char *)out.as.ptr, expected) != 0) {
        printf("FAIL %s: expected \"%s\", got ", name, expected);
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

    // === basic rescue ===
    ok &= test_int(L, "basic rescue",
        "begin; raise(\"oops\"); rescue; 42; end", 42);

    // === rescue => e ===
    ok &= test_str(L, "rescue => e",
        "begin; raise(\"hello\"); rescue => e; e; end", "hello");

    // === ensure runs on success ===
    ok &= test_int(L, "ensure on success",
        "x = 0; begin; x = 1; ensure; x = x + 10; end; x", 11);

    // === rescue + ensure ===
    ok &= test_int(L, "rescue + ensure",
        "x = 0; begin; raise(\"err\"); rescue; x = 1; ensure; x = x + 10; end; x", 11);

    // === ensure runs after rescue ===
    ok &= test_int(L, "ensure after rescue",
        "x = 0; begin; raise(\"err\"); rescue; x = 5; ensure; x = x + 100; end; x", 105);

    // === raise without rescue propagates ===
    ok &= test_error(L, "raise propagates",
        "raise(\"boom\")");

    // === begin as expression (body value) ===
    ok &= test_int(L, "begin as expression",
        "x = begin; 42; end; x", 42);

    // === rescue as expression ===
    ok &= test_int(L, "rescue as expression",
        "x = begin; raise(\"x\"); rescue; 99; end; x", 99);

    // === retry ===
    ok &= test_int(L, "retry",
        "x = 0; begin; x = x + 1; raise(\"err\") if x < 3; rescue; retry; end; x", 3);

    // === retry with rescue => e ===
    ok &= test_str(L, "retry with rescue => e",
        "x = 0; begin; x = x + 1; raise(\"try\" + x.to_s) if x < 2; rescue => e; retry; end; e", "try1");

    // === nested begin/rescue ===
    ok &= test_int(L, "nested begin/rescue",
        "begin; begin; raise(\"inner\"); rescue; 10; end; rescue; 20; end", 10);

    // === rescue => e message content ===
    ok &= test_str(L, "rescue error message",
        "begin; raise(\"specific error\"); rescue => e; e; end", "specific error");

    // === no error, rescue not executed ===
    ok &= test_int(L, "no error skips rescue",
        "begin; 42; rescue; 99; end", 42);

    // === ensure with no rescue ===
    ok &= test_int(L, "ensure without rescue",
        "x = 0; begin; x = 1; ensure; x = x + 10; end; x", 11);

    luby_free(L);
    printf("\nexceptions tests: %s\n", ok ? "ALL PASSED" : "SOME FAILED");
    return ok ? 0 : 1;
}
