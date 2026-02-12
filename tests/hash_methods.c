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

    // === each (generic, already works) ===
    ok &= test_int(L, "hash each",
        "sum = 0; {a: 1, b: 2, c: 3}.each { |k, v| sum = sum + v }; sum", 6);

    // === map on hash ===
    ok &= test_int(L, "hash map count",
        "a = {x: 1, y: 2}.map { |k, v| v * 10 }; len(a)", 2);
    ok &= test_int(L, "hash map values",
        "a = {x: 1, y: 2}.map { |k, v| v * 10 }; a[0] + a[1]", 30);

    // === select on hash ===
    ok &= test_int(L, "hash select",
        "h = {a: 1, b: 2, c: 3}.select { |k, v| v > 1 }; len(h.keys)", 2);

    // === reject on hash ===
    ok &= test_int(L, "hash reject",
        "h = {a: 1, b: 2, c: 3}.reject { |k, v| v > 1 }; len(h.keys)", 1);

    // === any? on hash ===
    ok &= test_bool(L, "hash any? true",
        "{a: 1, b: 5}.any? { |k, v| v > 3 }", 1);
    ok &= test_bool(L, "hash any? false",
        "{a: 1, b: 2}.any? { |k, v| v > 10 }", 0);

    // === all? on hash ===
    ok &= test_bool(L, "hash all? true",
        "{a: 1, b: 2}.all? { |k, v| v > 0 }", 1);
    ok &= test_bool(L, "hash all? false",
        "{a: 1, b: 2}.all? { |k, v| v > 1 }", 0);

    // === none? on hash ===
    ok &= test_bool(L, "hash none? true",
        "{a: 1, b: 2}.none? { |k, v| v > 10 }", 1);

    // === has_key? ===
    ok &= test_bool(L, "has_key? true",
        "{a: 1, b: 2}.has_key?(:a)", 1);
    ok &= test_bool(L, "has_key? false",
        "{a: 1, b: 2}.has_key?(:c)", 0);
    ok &= test_bool(L, "key? alias",
        "{a: 1, b: 2}.key?(:b)", 1);

    // === has_value? ===
    ok &= test_bool(L, "has_value? true",
        "{a: 1, b: 2}.has_value?(2)", 1);
    ok &= test_bool(L, "has_value? false",
        "{a: 1, b: 2}.has_value?(99)", 0);

    // === fetch ===
    ok &= test_int(L, "fetch found",
        "{a: 42, b: 7}.fetch(:a)", 42);
    ok &= test_int(L, "fetch with default",
        "{a: 42}.fetch(:z, 99)", 99);
    ok &= test_error(L, "fetch missing no default",
        "{a: 42}.fetch(:z)");

    // === delete ===
    ok &= test_int(L, "delete returns value",
        "h = {a: 1, b: 2}; h.delete(:a)", 1);
    ok &= test_int(L, "delete removes key",
        "h = {a: 1, b: 2}; h.delete(:a); len(h.keys)", 1);
    ok &= test_nil(L, "delete missing",
        "h = {a: 1}; h.delete(:z)");

    // === empty? ===
    ok &= test_bool(L, "hash empty? true",
        "{}.empty?", 1);
    ok &= test_bool(L, "hash empty? false",
        "{a: 1}.empty?", 0);
    ok &= test_bool(L, "array empty? true",
        "[].empty?", 1);
    ok &= test_bool(L, "array empty? false",
        "[1].empty?", 0);
    ok &= test_bool(L, "string empty? true",
        "\"\".empty?", 1);
    ok &= test_bool(L, "string empty? false",
        "\"hi\".empty?", 0);

    // === to_a ===
    ok &= test_int(L, "to_a count",
        "a = {x: 1, y: 2}.to_a; len(a)", 2);
    ok &= test_int(L, "to_a pair",
        "a = {x: 10}.to_a; a[0][1]", 10);

    // === merge ===
    ok &= test_int(L, "merge",
        "h = {a: 1}.merge({b: 2}); len(h.keys)", 2);
    ok &= test_int(L, "merge override",
        "h = {a: 1}.merge({a: 99}); h[:a]", 99);

    // === each_key / each_value ===
    ok &= test_int(L, "each_key",
        "c = 0; {a: 1, b: 2, c: 3}.each_key { |k| c = c + 1 }; c", 3);
    ok &= test_int(L, "each_value sum",
        "s = 0; {a: 10, b: 20}.each_value { |v| s = s + v }; s", 30);

    luby_free(L);
    printf("\nhash_methods tests: %s\n", ok ? "ALL PASSED" : "SOME FAILED");
    return ok ? 0 : 1;
}
