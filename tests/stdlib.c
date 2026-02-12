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

    // === sort_by ===
    ok &= test_int(L, "sort_by",
        "a = [3, 1, 2]; b = a.sort_by { |x| x }; b[0]", 1);
    ok &= test_int(L, "sort_by descending key",
        "a = [1, 3, 2]; b = a.sort_by { |x| 0 - x }; b[0]", 3);
    ok &= test_str(L, "sort_by strings by length",
        "a = [\"bb\", \"a\", \"ccc\"]; b = a.sort_by { |x| len(x) }; b[0]", "a");

    // === min_by ===
    ok &= test_int(L, "min_by",
        "a = [3, 1, 2]; a.min_by { |x| x }", 1);
    ok &= test_int(L, "min_by negative key",
        "a = [3, 1, 2]; a.min_by { |x| 0 - x }", 3);

    // === max_by ===
    ok &= test_int(L, "max_by",
        "a = [3, 1, 2]; a.max_by { |x| x }", 3);
    ok &= test_str(L, "max_by string length",
        "a = [\"a\", \"ccc\", \"bb\"]; a.max_by { |x| len(x) }", "ccc");

    // === group_by ===
    ok &= test_int(L, "group_by even/odd",
        "a = [1, 2, 3, 4, 5]; g = a.group_by { |x| x % 2 }; len(g[1])", 3);
    ok &= test_int(L, "group_by even count",
        "a = [1, 2, 3, 4, 5]; g = a.group_by { |x| x % 2 }; len(g[0])", 2);

    // === flat_map ===
    ok &= test_int(L, "flat_map",
        "a = [1, 2, 3]; b = a.flat_map { |x| [x, x * 10] }; len(b)", 6);
    ok &= test_int(L, "flat_map values",
        "a = [1, 2, 3]; b = a.flat_map { |x| [x, x * 10] }; b[0] + b[1] + b[2] + b[3]", 33);
    ok &= test_int(L, "flat_map non-array",
        "a = [1, 2, 3]; b = a.flat_map { |x| x * 2 }; b[0] + b[1] + b[2]", 12);

    // === sum ===
    ok &= test_int(L, "sum",
        "[1, 2, 3, 4].sum", 10);
    ok &= test_int(L, "sum with block",
        "[1, 2, 3].sum { |x| x * x }", 14);
    ok &= test_int(L, "sum empty",
        "[].sum", 0);

    // === count ===
    ok &= test_int(L, "count no block",
        "[1, 2, 3, 4, 5].count", 5);
    ok &= test_int(L, "count with block",
        "[1, 2, 3, 4, 5].count { |x| x > 3 }", 2);
    ok &= test_int(L, "count none match",
        "[1, 2, 3].count { |x| x > 10 }", 0);

    // === zip ===
    ok &= test_int(L, "zip length",
        "a = [1, 2, 3]; b = [4, 5, 6]; c = a.zip(b); len(c)", 3);
    ok &= test_int(L, "zip values",
        "a = [1, 2]; b = [10, 20]; c = a.zip(b); c[0][0] + c[0][1]", 11);
    ok &= test_int(L, "zip unequal length",
        "a = [1, 2, 3]; b = [10]; c = a.zip(b); c[2][0]", 3);

    // === gsub ===
    ok &= test_str(L, "gsub",
        "\"hello world\".gsub(\"o\", \"0\")", "hell0 w0rld");
    ok &= test_str(L, "gsub no match",
        "\"hello\".gsub(\"xyz\", \"!\")", "hello");
    ok &= test_str(L, "gsub multi-char",
        "\"aabbaabb\".gsub(\"bb\", \"X\")", "aaXaaX");

    // === sub ===
    ok &= test_str(L, "sub first only",
        "\"hello hello\".sub(\"hello\", \"hi\")", "hi hello");
    ok &= test_str(L, "sub no match",
        "\"hello\".sub(\"xyz\", \"!\")", "hello");

    // === start_with? ===
    ok &= test_bool(L, "start_with? true",
        "\"hello world\".start_with?(\"hello\")", 1);
    ok &= test_bool(L, "start_with? false",
        "\"hello world\".start_with?(\"world\")", 0);

    // === end_with? ===
    ok &= test_bool(L, "end_with? true",
        "\"hello world\".end_with?(\"world\")", 1);
    ok &= test_bool(L, "end_with? false",
        "\"hello world\".end_with?(\"hello\")", 0);

    luby_free(L);
    printf("\nstdlib tests: %s\n", ok ? "ALL PASSED" : "SOME FAILED");
    return ok ? 0 : 1;
}
