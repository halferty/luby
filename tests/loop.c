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

static void test_loop_with_do(void) {
    printf("Testing: loop do ... end\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "i = 0\n"
        "loop do\n"
        "  i = i + 1\n"
        "  if i >= 5\n"
        "    break\n"
        "  end\n"
        "end\n"
        "i\n";
    if (eval_check(L, "loop do", code, &result)) {
        assert_int("loop do result", result, 5);
    }
    luby_free(L);
}

static void test_loop_without_do(void) {
    printf("Testing: loop ... end (no do)\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "i = 0\n"
        "loop\n"
        "  i = i + 1\n"
        "  if i >= 3\n"
        "    break\n"
        "  end\n"
        "end\n"
        "i\n";
    if (eval_check(L, "loop no do", code, &result)) {
        assert_int("loop no do result", result, 3);
    }
    luby_free(L);
}

static void test_loop_with_next(void) {
    printf("Testing: loop with next\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "i = 0\n"
        "sum = 0\n"
        "loop do\n"
        "  i = i + 1\n"
        "  if i >= 10\n"
        "    break\n"
        "  end\n"
        "  if i % 2 == 0\n"
        "    next\n"
        "  end\n"
        "  sum = sum + i\n"
        "end\n"
        "sum\n";  // 1 + 3 + 5 + 7 + 9 = 25
    if (eval_check(L, "loop next", code, &result)) {
        assert_int("loop next result", result, 25);
    }
    luby_free(L);
}

static void test_nested_loops(void) {
    printf("Testing: nested loops\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "count = 0\n"
        "i = 0\n"
        "loop do\n"
        "  i = i + 1\n"
        "  j = 0\n"
        "  loop do\n"
        "    j = j + 1\n"
        "    count = count + 1\n"
        "    if j >= 3\n"
        "      break\n"
        "    end\n"
        "  end\n"
        "  if i >= 4\n"
        "    break\n"
        "  end\n"
        "end\n"
        "count\n";  // 4 outer iterations * 3 inner iterations = 12
    if (eval_check(L, "nested loops", code, &result)) {
        assert_int("nested loops result", result, 12);
    }
    luby_free(L);
}

int main(void) {
    printf("=== Loop Keyword Tests ===\n\n");

    test_loop_with_do();
    test_loop_without_do();
    test_loop_with_next();
    test_nested_loops();

    printf("\n=== Summary: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
