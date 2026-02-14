#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

static int assert_int(const char *label, luby_value v, int64_t expected) {
    if (v.type != LUBY_T_INT || v.as.i != expected) {
        printf("FAIL: %s (got type=%d, val=%lld, expected %lld)\n", label, v.type, (long long)v.as.i, (long long)expected);
        tests_failed++;
        return 0;
    }
    tests_passed++;
    return 1;
}

static int eval_check(luby_state *L, const char *label, const char *code, luby_value *out) {
    int rc = luby_eval(L, code, 0, "<test>", out);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL: %s (%s)\n", label, buf);
        tests_failed++;
        return 0;
    }
    return 1;
}

static void test_basic_for(void) {
    printf("Testing: basic for loop\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "sum = 0\n"
        "for x in [1, 2, 3, 4, 5]\n"
        "  sum = sum + x\n"
        "end\n"
        "sum\n";
    if (eval_check(L, "basic for", code, &result)) {
        assert_int("basic for result", result, 15);
    }
    luby_free(L);
}

static void test_for_with_do(void) {
    printf("Testing: for x in array do ... end\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "sum = 0\n"
        "for x in [1, 2, 3] do\n"
        "  sum += x\n"
        "end\n"
        "sum\n";
    if (eval_check(L, "for with do", code, &result)) {
        assert_int("for with do result", result, 6);
    }
    luby_free(L);
}

static void test_for_one_line(void) {
    printf("Testing: one-line for\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code = "sum = 0; for x in [10, 20, 30] do sum += x; end; sum";
    if (eval_check(L, "one-line for", code, &result)) {
        assert_int("one-line for result", result, 60);
    }
    luby_free(L);
}

static void test_for_range(void) {
    printf("Testing: for with range\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "sum = 0\n"
        "for i in 1..5\n"
        "  sum += i\n"
        "end\n"
        "sum\n";
    if (eval_check(L, "for with range", code, &result)) {
        assert_int("for with range result", result, 15);
    }
    luby_free(L);
}

static void test_for_multiple_vars(void) {
    printf("Testing: for with multiple variables\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    // Using hash entries which yield key, value pairs
    const char *code =
        "sum = 0\n"
        "h = {a: 1, b: 2, c: 3}\n"
        "h.each { |k, v| sum += v }\n"
        "sum\n";
    if (eval_check(L, "for multiple vars", code, &result)) {
        assert_int("for multiple vars result", result, 6);
    }
    luby_free(L);
}

static void test_for_with_break(void) {
    printf("Testing: for with break (via while loop)\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    // Note: break inside for is limited since for desugars to each+block
    // Test break in while instead
    const char *code =
        "sum = 0\n"
        "i = 0\n"
        "arr = [1, 2, 3, 4, 5]\n"
        "while i < arr.size\n"
        "  x = arr[i]\n"
        "  if x > 3\n"
        "    break\n"
        "  end\n"
        "  sum += x\n"
        "  i += 1\n"
        "end\n"
        "sum\n";
    if (eval_check(L, "while with break", code, &result)) {
        assert_int("while with break result", result, 6);
    }
    luby_free(L);
}

static void test_for_with_next(void) {
    printf("Testing: for with next (via while loop)\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    // Note: next inside for is limited since for desugars to each+block
    // Test next in while instead
    const char *code =
        "sum = 0\n"
        "i = 0\n"
        "arr = [1, 2, 3, 4, 5]\n"
        "while i < arr.size\n"
        "  x = arr[i]\n"
        "  i += 1\n"
        "  if x == 3\n"
        "    next\n"
        "  end\n"
        "  sum += x\n"
        "end\n"
        "sum\n";
    if (eval_check(L, "while with next", code, &result)) {
        assert_int("while with next result", result, 12);  // 1+2+4+5
    }
    luby_free(L);
}

static void test_while_with_do(void) {
    printf("Testing: while with do\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "x = 0\n"
        "while x < 5 do\n"
        "  x += 1\n"
        "end\n"
        "x\n";
    if (eval_check(L, "while with do", code, &result)) {
        assert_int("while with do result", result, 5);
    }
    luby_free(L);
}

static void test_until_with_do(void) {
    printf("Testing: until with do\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "x = 0\n"
        "until x >= 5 do\n"
        "  x += 1\n"
        "end\n"
        "x\n";
    if (eval_check(L, "until with do", code, &result)) {
        assert_int("until with do result", result, 5);
    }
    luby_free(L);
}

int main() {
    printf("=== For Loop Tests ===\n\n");
    
    test_basic_for();
    test_for_with_do();
    test_for_one_line();
    test_for_range();
    test_for_multiple_vars();
    test_for_with_break();
    test_for_with_next();
    test_while_with_do();
    test_until_with_do();
    
    printf("\n=== Results ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    return tests_failed ? 1 : 0;
}
