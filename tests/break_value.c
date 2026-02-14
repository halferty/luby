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

static int assert_nil(const char *label, luby_value v) {
    if (v.type != LUBY_T_NIL) {
        printf("FAIL: %s (got type=%d, expected nil)\n", label, v.type);
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

static void test_break_with_value(void) {
    printf("Testing: break with value\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "i = 0\n"
        "while true\n"
        "  i = i + 1\n"
        "  if i >= 3\n"
        "    break 42\n"
        "  end\n"
        "end\n";
    if (eval_check(L, "break with value", code, &result)) {
        assert_int("break value result", result, 42);
    }
    luby_free(L);
}

static void test_break_with_expression(void) {
    printf("Testing: break with expression\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "i = 0\n"
        "while true\n"
        "  i = i + 1\n"
        "  if i >= 5\n"
        "    break i * 10\n"
        "  end\n"
        "end\n";
    if (eval_check(L, "break with expression", code, &result)) {
        assert_int("break expr result", result, 50);
    }
    luby_free(L);
}

static void test_break_value_if_modifier(void) {
    printf("Testing: break value if modifier\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "i = 0\n"
        "while true\n"
        "  i = i + 1\n"
        "  break 99 if i >= 3\n"
        "end\n";
    if (eval_check(L, "break if modifier", code, &result)) {
        assert_int("break if modifier result", result, 99);
    }
    luby_free(L);
}

static void test_break_value_unless_modifier(void) {
    printf("Testing: break value unless modifier\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "i = 0\n"
        "while true\n"
        "  i = i + 1\n"
        "  break 77 unless i < 4\n"
        "end\n";
    if (eval_check(L, "break unless modifier", code, &result)) {
        assert_int("break unless modifier result", result, 77);
    }
    luby_free(L);
}

static void test_break_no_value(void) {
    printf("Testing: break without value returns nil\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "i = 0\n"
        "while true\n"
        "  i = i + 1\n"
        "  break if i >= 2\n"
        "end\n";
    if (eval_check(L, "break no value", code, &result)) {
        assert_nil("break no value result", result);
    }
    luby_free(L);
}

static void test_next_with_value_in_loop(void) {
    printf("Testing: next with value in loop\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    // next with value doesn't affect the loop result, just continues
    // but we can verify the loop completes correctly
    const char *code =
        "sum = 0\n"
        "i = 0\n"
        "while i < 5\n"
        "  i = i + 1\n"
        "  next 0 if i == 3\n"  // skip adding 3
        "  sum = sum + i\n"
        "end\n"
        "sum\n";  // 1 + 2 + 4 + 5 = 12
    if (eval_check(L, "next with value", code, &result)) {
        assert_int("next value result", result, 12);
    }
    luby_free(L);
}

static void test_break_in_loop_keyword(void) {
    printf("Testing: break value in loop keyword\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "i = 0\n"
        "loop do\n"
        "  i = i + 1\n"
        "  break i * 100 if i >= 2\n"
        "end\n";
    if (eval_check(L, "break in loop", code, &result)) {
        assert_int("break in loop result", result, 200);
    }
    luby_free(L);
}

int main(void) {
    printf("=== Break/Next with Value Tests ===\n\n");

    test_break_with_value();
    test_break_with_expression();
    test_break_value_if_modifier();
    test_break_value_unless_modifier();
    test_break_no_value();
    test_next_with_value_in_loop();
    test_break_in_loop_keyword();

    printf("\n=== Summary: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
