#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>
#include <string.h>

static int pass_count = 0;
static int fail_count = 0;

static void check(const char *label, int ok) {
    if (ok) { pass_count++; printf("  PASS: %s\n", label); }
    else    { fail_count++; printf("  FAIL: %s\n", label); }
}

static void test_method_in_def(void) {
    printf("== __method__ inside a def ==\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value out;
    int r = luby_eval(L,
        "def foo\n"
        "  __method__\n"
        "end\n"
        "foo()\n", 0, "test", &out);
    check("exec ok", r == 0);
    check("returns symbol", out.type == LUBY_T_SYMBOL);
    if (out.type == LUBY_T_SYMBOL && out.as.ptr) {
        check("value is :foo", strcmp((const char *)out.as.ptr, "foo") == 0);
    } else {
        check("value is :foo", 0);
    }
    luby_free(L);
}

static void test_method_in_toplevel(void) {
    printf("== __method__ at top level ==\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value out;
    int r = luby_eval(L, "__method__\n", 0, "test", &out);
    check("exec ok", r == 0);
    /* At top level there is no method name, should be nil */
    check("returns nil at top level", out.type == LUBY_T_NIL);
    luby_free(L);
}

static void test_callee_in_def(void) {
    printf("== __callee__ inside a def ==\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value out;
    int r = luby_eval(L,
        "def bar\n"
        "  __callee__\n"
        "end\n"
        "bar()\n", 0, "test", &out);
    check("exec ok", r == 0);
    check("returns symbol", out.type == LUBY_T_SYMBOL);
    if (out.type == LUBY_T_SYMBOL && out.as.ptr) {
        check("value is :bar", strcmp((const char *)out.as.ptr, "bar") == 0);
    } else {
        check("value is :bar", 0);
    }
    luby_free(L);
}

static void test_method_in_class(void) {
    printf("== __method__ inside a class method ==\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value out;
    int r = luby_eval(L,
        "class Dog\n"
        "  def speak\n"
        "    __method__\n"
        "  end\n"
        "end\n"
        "Dog.new.speak\n", 0, "test", &out);
    check("exec ok", r == 0);
    check("returns symbol", out.type == LUBY_T_SYMBOL);
    if (out.type == LUBY_T_SYMBOL && out.as.ptr) {
        check("value is :speak", strcmp((const char *)out.as.ptr, "speak") == 0);
    } else {
        check("value is :speak", 0);
    }
    luby_free(L);
}

static void test_caller_basic(void) {
    printf("== caller returns an array ==\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value out;
    int r = luby_eval(L,
        "def baz\n"
        "  caller()\n"
        "end\n"
        "baz()\n", 0, "test", &out);
    check("exec ok", r == 0);
    check("returns array", out.type == LUBY_T_ARRAY);
    if (out.type == LUBY_T_ARRAY) {
        size_t len = luby_array_len(out);
        check("array not empty", len > 0);
        /* First element should be a string containing the filename */
        if (len > 0) {
            luby_value elem; luby_array_get(out, 0, &elem);
            check("element is string", elem.type == LUBY_T_STRING);
            if (elem.type == LUBY_T_STRING && elem.as.ptr) {
                const char *s = (const char *)elem.as.ptr;
                /* Frame filename can be <proc> or test */
                check("element is a caller string", strlen(s) > 0);
            } else {
                check("element is a caller string", 0);
            }
        }
    } else {
        check("array not empty", 0);
        check("element is string", 0);
        check("contains 'test'", 0);
    }
    luby_free(L);
}

static void test_caller_depth(void) {
    printf("== caller shows call depth ==\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value out;
    int r = luby_eval(L,
        "def inner\n"
        "  caller()\n"
        "end\n"
        "def outer\n"
        "  inner()\n"
        "end\n"
        "outer()\n", 0, "test", &out);
    check("exec ok", r == 0);
    check("returns array", out.type == LUBY_T_ARRAY);
    if (out.type == LUBY_T_ARRAY) {
        size_t len = luby_array_len(out);
        check("at least 2 frames", len >= 2);
    } else {
        check("at least 2 frames", 0);
    }
    luby_free(L);
}

static void test_method_to_s(void) {
    printf("== __method__.to_s ==\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value out;
    int r = luby_eval(L,
        "def greet\n"
        "  __method__.to_s\n"
        "end\n"
        "greet()\n", 0, "test", &out);
    check("exec ok", r == 0);
    check("returns string", out.type == LUBY_T_STRING);
    if (out.type == LUBY_T_STRING && out.as.ptr) {
        check("value is 'greet'", strcmp((const char *)out.as.ptr, "greet") == 0);
    } else {
        check("value is 'greet'", 0);
    }
    luby_free(L);
}

int main(void) {
    printf("=== __method__ / __callee__ / caller tests ===\n");
    test_method_in_def();
    test_method_in_toplevel();
    test_callee_in_def();
    test_method_in_class();
    test_caller_basic();
    test_caller_depth();
    test_method_to_s();
    printf("\n%d passed, %d failed\n", pass_count, fail_count);
    return fail_count ? 1 : 0;
}
