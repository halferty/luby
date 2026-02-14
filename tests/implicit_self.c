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

static void test_basic_implicit_self(void) {
    printf("Testing: basic implicit self method call\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "class C\n"
        "  def helper\n"
        "    42\n"
        "  end\n"
        "  def test\n"
        "    helper\n"
        "  end\n"
        "end\n"
        "C.new.test\n";
    if (eval_check(L, "basic implicit self", code, &result)) {
        assert_int("basic implicit self", result, 42);
    }
    luby_free(L);
}

static void test_chain_implicit_self(void) {
    printf("Testing: chained implicit self calls\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "class C\n"
        "  def a\n"
        "    10\n"
        "  end\n"
        "  def b\n"
        "    a + 5\n"
        "  end\n"
        "  def c\n"
        "    b * 2\n"
        "  end\n"
        "end\n"
        "C.new.c\n";
    if (eval_check(L, "chained implicit self", code, &result)) {
        assert_int("chained implicit self (10+5)*2=30", result, 30);
    }
    luby_free(L);
}

static void test_implicit_self_in_inherited_class(void) {
    printf("Testing: implicit self in inherited class\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "class Parent\n"
        "  def parent_method\n"
        "    100\n"
        "  end\n"
        "end\n"
        "class Child < Parent\n"
        "  def test\n"
        "    parent_method\n"
        "  end\n"
        "end\n"
        "Child.new.test\n";
    if (eval_check(L, "implicit self in inherited class", code, &result)) {
        assert_int("implicit self in inherited class", result, 100);
    }
    luby_free(L);
}

static void test_implicit_self_with_include(void) {
    printf("Testing: implicit self with included module\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "module M\n"
        "  def module_method\n"
        "    200\n"
        "  end\n"
        "end\n"
        "class C\n"
        "  include M\n"
        "  def test\n"
        "    module_method\n"
        "  end\n"
        "end\n"
        "C.new.test\n";
    if (eval_check(L, "implicit self with include", code, &result)) {
        assert_int("implicit self with include", result, 200);
    }
    luby_free(L);
}

static void test_implicit_self_private_method(void) {
    printf("Testing: implicit self with private method\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "class C\n"
        "  def test\n"
        "    helper\n"
        "  end\n"
        "  private\n"
        "  def helper\n"
        "    300\n"
        "  end\n"
        "end\n"
        "C.new.test\n";
    if (eval_check(L, "implicit self private method", code, &result)) {
        assert_int("implicit self private method", result, 300);
    }
    luby_free(L);
}

static void test_implicit_self_with_args(void) {
    printf("Testing: implicit self method with arguments\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "class Calculator\n"
        "  def add(a, b)\n"
        "    a + b\n"
        "  end\n"
        "  def multiply(a, b)\n"
        "    a * b\n"
        "  end\n"
        "  def compute\n"
        "    add(3, 4) + multiply(2, 5)\n"
        "  end\n"
        "end\n"
        "Calculator.new.compute\n";
    if (eval_check(L, "implicit self with args", code, &result)) {
        assert_int("implicit self with args (3+4)+(2*5)=17", result, 17);
    }
    luby_free(L);
}

static void test_implicit_self_vs_local(void) {
    printf("Testing: local variable shadows implicit self\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    // When a local variable exists with the same name, it should take precedence
    const char *code =
        "class C\n"
        "  def helper\n"
        "    999\n"
        "  end\n"
        "  def test\n"
        "    helper = 42\n"
        "    helper\n"
        "  end\n"
        "end\n"
        "C.new.test\n";
    if (eval_check(L, "local shadows implicit self", code, &result)) {
        assert_int("local shadows implicit self", result, 42);
    }
    luby_free(L);
}

static void test_recursive_implicit_self(void) {
    printf("Testing: recursive implicit self call\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "class C\n"
        "  def factorial(n)\n"
        "    if n <= 1\n"
        "      1\n"
        "    else\n"
        "      n * factorial(n - 1)\n"
        "    end\n"
        "  end\n"
        "end\n"
        "C.new.factorial(5)\n";
    if (eval_check(L, "recursive implicit self", code, &result)) {
        assert_int("recursive implicit self (5! = 120)", result, 120);
    }
    luby_free(L);
}

int main(void) {
    printf("=== Implicit Self Method Call Tests ===\n\n");
    
    test_basic_implicit_self();
    test_chain_implicit_self();
    test_implicit_self_in_inherited_class();
    test_implicit_self_with_include();
    test_implicit_self_private_method();
    test_implicit_self_with_args();
    test_implicit_self_vs_local();
    test_recursive_implicit_self();
    
    printf("\n=== Summary: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
