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

static void test_module_function_explicit(void) {
    printf("Testing: module_function :method_name\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "module M\n"
        "  def helper\n"
        "    42\n"
        "  end\n"
        "  module_function :helper\n"
        "end\n"
        "M.helper\n";
    if (eval_check(L, "module_function :method_name", code, &result)) {
        assert_int("module_function :method_name", result, 42);
    }
    luby_free(L);
}

static void test_module_function_mode(void) {
    printf("Testing: module_function mode (no args)\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "module M\n"
        "  module_function\n"
        "  def foo\n"
        "    100\n"
        "  end\n"
        "  def bar\n"
        "    200\n"
        "  end\n"
        "end\n"
        "M.foo + M.bar\n";
    if (eval_check(L, "module_function mode", code, &result)) {
        assert_int("module_function mode", result, 300);
    }
    luby_free(L);
}

static void test_module_function_include(void) {
    printf("Testing: module_function methods can be called via include\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "module M\n"
        "  def compute\n"
        "    123\n"
        "  end\n"
        "  module_function :compute\n"
        "end\n"
        "class C\n"
        "  include M\n"
        "  def test\n"
        "    compute\n"
        "  end\n"
        "end\n"
        "C.new.test\n";
    if (eval_check(L, "module_function include", code, &result)) {
        assert_int("module_function include", result, 123);
    }
    luby_free(L);
}

static void test_module_function_singleton_call(void) {
    printf("Testing: module_function creates singleton method\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "module Calculator\n"
        "  def add(a, b)\n"
        "    a + b\n"
        "  end\n"
        "  module_function :add\n"
        "end\n"
        "Calculator.add(10, 5)\n";
    if (eval_check(L, "module_function singleton", code, &result)) {
        assert_int("module_function singleton", result, 15);
    }
    luby_free(L);
}

static void test_module_function_mode_ends_at_module_end(void) {
    printf("Testing: module_function mode ends when module ends\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "module M1\n"
        "  module_function\n"
        "  def helper\n"
        "    1\n"
        "  end\n"
        "end\n"
        "module M2\n"
        "  def regular\n"
        "    2\n"
        "  end\n"
        "end\n"
        "M1.helper\n";
    if (eval_check(L, "module_function mode ends", code, &result)) {
        assert_int("module_function mode ends", result, 1);
    }
    luby_free(L);
}

static void test_module_function_math_example(void) {
    printf("Testing: module_function Math-style usage\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "module MyMath\n"
        "  module_function\n"
        "  def square(x)\n"
        "    x * x\n"
        "  end\n"
        "  def cube(x)\n"
        "    x * x * x\n"
        "  end\n"
        "end\n"
        "MyMath.square(3) + MyMath.cube(2)\n";
    if (eval_check(L, "module_function math", code, &result)) {
        assert_int("module_function math (9+8)", result, 17);
    }
    luby_free(L);
}

int main(void) {
    printf("=== Module Function Tests ===\n\n");
    
    test_module_function_explicit();
    test_module_function_mode();
    test_module_function_include();
    test_module_function_singleton_call();
    test_module_function_mode_ends_at_module_end();
    test_module_function_math_example();
    
    printf("\n=== Summary: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
