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

    // === to_a ===
    ok &= test_int(L, "to_a inclusive count",
        "a = (1..5).to_a; len(a)", 5);
    ok &= test_int(L, "to_a inclusive first",
        "(1..5).to_a[0]", 1);
    ok &= test_int(L, "to_a inclusive last",
        "(1..5).to_a[4]", 5);
    ok &= test_int(L, "to_a exclusive count",
        "a = (1...5).to_a; len(a)", 4);
    ok &= test_int(L, "to_a exclusive last",
        "(1...5).to_a[3]", 4);

    // === size / count ===
    ok &= test_int(L, "size inclusive",
        "(1..10).size", 10);
    ok &= test_int(L, "size exclusive",
        "(1...10).size", 9);
    ok &= test_int(L, "count",
        "(1..5).count", 5);
    ok &= test_int(L, "length",
        "(1..3).length", 3);
    ok &= test_int(L, "len on range",
        "len(1..5)", 5);

    // === include? ===
    ok &= test_bool(L, "include? true",
        "(1..10).include?(5)", 1);
    ok &= test_bool(L, "include? false",
        "(1..10).include?(11)", 0);
    ok &= test_bool(L, "include? exclusive boundary",
        "(1...5).include?(5)", 0);
    ok &= test_bool(L, "include? exclusive inside",
        "(1...5).include?(4)", 1);

    // === min / max ===
    ok &= test_int(L, "min",
        "(3..7).min", 3);
    ok &= test_int(L, "max inclusive",
        "(3..7).max", 7);
    ok &= test_int(L, "max exclusive",
        "(3...7).max", 6);

    // === first / last ===
    ok &= test_int(L, "first",
        "(1..5).first", 1);
    ok &= test_int(L, "last inclusive",
        "(1..5).last", 5);
    ok &= test_int(L, "last exclusive",
        "(1...5).last", 4);
    ok &= test_int(L, "first(n) count",
        "len((1..10).first(3))", 3);
    ok &= test_int(L, "first(n) value",
        "(1..10).first(3)[2]", 3);
    ok &= test_int(L, "last(n) count",
        "len((1..10).last(2))", 2);
    ok &= test_int(L, "last(n) value",
        "(1..10).last(2)[0]", 9);

    // === each ===
    ok &= test_int(L, "each sum",
        "s = 0; (1..5).each { |x| s = s + x }; s", 15);

    // === for-in ===
    ok &= test_int(L, "for-in loop",
        "s = 0; for i in 1..5; s = s + i; end; s", 15);

    // === map ===
    ok &= test_int(L, "map count",
        "len((1..5).map { |x| x * 2 })", 5);
    ok &= test_int(L, "map values",
        "a = (1..3).map { |x| x * 10 }; a[0] + a[1] + a[2]", 60);

    // === select ===
    ok &= test_int(L, "select count",
        "len((1..10).select { |x| x % 2 == 0 })", 5);
    ok &= test_int(L, "select first",
        "(1..10).select { |x| x % 2 == 0 }[0]", 2);

    // === reject ===
    ok &= test_int(L, "reject count",
        "len((1..10).reject { |x| x % 2 == 0 })", 5);
    ok &= test_int(L, "reject first",
        "(1..10).reject { |x| x % 2 == 0 }[0]", 1);

    // === any? / all? / none? ===
    ok &= test_bool(L, "any? true",
        "(1..5).any? { |x| x > 3 }", 1);
    ok &= test_bool(L, "any? false",
        "(1..5).any? { |x| x > 10 }", 0);
    ok &= test_bool(L, "all? true",
        "(1..5).all? { |x| x > 0 }", 1);
    ok &= test_bool(L, "all? false",
        "(1..5).all? { |x| x > 3 }", 0);
    ok &= test_bool(L, "none? true",
        "(1..5).none? { |x| x > 10 }", 1);
    ok &= test_bool(L, "none? false",
        "(1..5).none? { |x| x > 3 }", 0);

    // === sum ===
    ok &= test_int(L, "sum",
        "(1..100).sum", 5050);
    ok &= test_int(L, "sum with block",
        "(1..3).sum { |x| x * x }", 14);

    // === step ===
    ok &= test_int(L, "step array count",
        "len((1..10).step(3))", 4);
    ok &= test_int(L, "step array values",
        "a = (1..10).step(3); a[0] + a[1] + a[2] + a[3]", 22);
    ok &= test_int(L, "step with block",
        "s = 0; (1..10).step(2) { |x| s = s + x }; s", 25);

    // === reverse_each ===
    ok &= test_int(L, "reverse_each",
        "a = []; (1..3).reverse_each { |x| a = a + [x] }; a[0]", 3);
    ok &= test_int(L, "reverse_each last",
        "a = []; (1..3).reverse_each { |x| a = a + [x] }; a[2]", 1);

    // === Array slicing with ranges ===
    ok &= test_int(L, "array slice inclusive count",
        "len([10, 20, 30, 40, 50][1..3])", 3);
    ok &= test_int(L, "array slice inclusive values",
        "a = [10, 20, 30, 40, 50][1..3]; a[0] + a[1] + a[2]", 90);
    ok &= test_int(L, "array slice exclusive count",
        "len([10, 20, 30, 40, 50][1...3])", 2);
    ok &= test_int(L, "array slice exclusive values",
        "a = [10, 20, 30, 40, 50][1...3]; a[0] + a[1]", 50);

    // === String slicing with ranges ===
    ok &= test_str(L, "string slice inclusive",
        "\"hello\"[1..3]", "ell");
    ok &= test_str(L, "string slice exclusive",
        "\"hello\"[1...3]", "el");

    // === case/when with ranges ===
    ok &= test_int(L, "case/when range match",
        "case 5; when 1..3; 1; when 4..6; 2; when 7..9; 3; end", 2);
    ok &= test_int(L, "case/when range first",
        "case 2; when 1..3; 10; when 4..6; 20; end", 10);
    ok &= test_int(L, "case/when range exclusive",
        "case 5; when 1...5; 1; when 5..10; 2; end", 2);

    // === empty? ===
    ok &= test_bool(L, "empty? false",
        "(1..5).empty?", 0);
    ok &= test_bool(L, "empty? true (inverted range)",
        "(5..1).empty?", 1);

    luby_free(L);
    printf("\nrange tests: %s\n", ok ? "ALL PASSED" : "SOME FAILED");
    return ok ? 0 : 1;
}
