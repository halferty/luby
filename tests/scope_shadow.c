#include <stdio.h>
#include <string.h>
#define LUBY_IMPLEMENTATION
#include "../luby.h"

static int tests_run = 0, tests_passed = 0;

static int assert_int(const char *label, luby_value v, int64_t expected) {
    if (v.type != LUBY_T_INT || v.as.i != expected) {
        printf("FAIL: %s (got type=%d val=%lld, expected %lld)\n",
               label, v.type, (long long)v.as.i, (long long)expected);
        return 0;
    }
    return 1;
}

static int assert_string(const char *label, luby_value v, const char *expected) {
    if (v.type != LUBY_T_STRING) {
        printf("FAIL: %s (got type=%d, expected string)\n", label, v.type);
        return 0;
    }
    const char *actual = v.as.ptr ? (const char *)v.as.ptr : "";
    if (strcmp(actual, expected) != 0) {
        printf("FAIL: %s (got \"%s\", expected \"%s\")\n", label, actual, expected);
        return 0;
    }
    return 1;
}

static int assert_nil(const char *label, luby_value v) {
    if (v.type != LUBY_T_NIL) {
        printf("FAIL: %s (got type=%d, expected nil)\n", label, v.type);
        return 0;
    }
    return 1;
}

static int eval_ok(luby_state *L, const char *label, const char *code, luby_value *out) {
    int rc = luby_eval(L, code, 0, "<test>", out);
    if (rc != 0) {
        char buf[512];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL: %s — error: %s\n", label, buf);
        return 0;
    }
    return 1;
}

static luby_state *new_state(void) {
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    return L;
}

static void test(const char *name, int ok) {
    tests_run++;
    if (ok) { tests_passed++; printf("  PASS: %s\n", name); }
    else     { printf("  FAIL: %s\n", name); }
}

int main(void) {
    printf("=== Scope & Shadow Tests ===\n\n");

    /* ---- Bug #1: Variable scoping leak ---- */
    printf("-- Variable Scoping --\n");

    {
        /* Test that a variable assigned ONLY inside a function doesn't 
           persist in the global table after the function returns */
        luby_state *L = new_state();
        luby_value v;
        int ok = eval_ok(L, "local doesn't leak", 
            "def f()\n"
            "  secret = 42\n"
            "  secret\n"
            "end\n"
            "f()\n", &v)
          && assert_int("f() returns 42", v, 42);
        if (ok) {
            /* Now set secret to something else and call f() again — 
               if scoping works, secret shouldn't persist between calls */
            luby_value v2;
            ok = eval_ok(L, "secret not leaked",
                "secret = 0\n"
                "f()\n"
                "secret\n", &v2)
              && assert_int("secret is 0 (not leaked 42)", v2, 0);
        }
        test("local doesn't leak out", ok);
        luby_free(L);
    }

    {
        luby_state *L = new_state();
        luby_value v;
        int ok = eval_ok(L, "caller var not clobbered",
            "x = 10\n"
            "def f()\n"
            "  x = 99\n"
            "  x\n"
            "end\n"
            "f()\n"
            "x\n", &v)
          && assert_int("caller x intact", v, 10);
        test("caller var not clobbered", ok);
        luby_free(L);
    }

    {
        luby_state *L = new_state();
        luby_value v;
        int ok = eval_ok(L, "function returns its own local",
            "def f()\n"
            "  x = 42\n"
            "  x\n"
            "end\n"
            "f()\n", &v)
          && assert_int("f() == 42", v, 42);
        test("function returns its own local", ok);
        luby_free(L);
    }

    {
        luby_state *L = new_state();
        luby_value v;
        int ok = eval_ok(L, "nested calls don't interfere",
            "def inner()\n"
            "  t = 100\n"
            "  t\n"
            "end\n"
            "def outer()\n"
            "  t = 1\n"
            "  r = inner()\n"
            "  t + r\n"
            "end\n"
            "outer()\n", &v)
          && assert_int("outer() == 101", v, 101);
        test("nested calls don't interfere", ok);
        luby_free(L);
    }

    {
        luby_state *L = new_state();
        luby_value v;
        int ok = eval_ok(L, "recursive locals are independent",
            "def fact(n)\n"
            "  if n <= 1\n"
            "    result = 1\n"
            "  else\n"
            "    sub = fact(n - 1)\n"
            "    result = n * sub\n"
            "  end\n"
            "  result\n"
            "end\n"
            "fact(5)\n", &v)
          && assert_int("fact(5) == 120", v, 120);
        test("recursive locals are independent", ok);
        luby_free(L);
    }

    {
        luby_state *L = new_state();
        luby_value v;
        int ok = eval_ok(L, "multi-assign locals scoped",
            "def f()\n"
            "  a, b = 10, 20\n"
            "  a + b\n"
            "end\n"
            "a = 1\n"
            "b = 2\n"
            "f()\n"
            "a + b\n", &v)
          && assert_int("a+b == 3", v, 3);
        test("multi-assign locals scoped", ok);
        luby_free(L);
    }

    {
        luby_state *L = new_state();
        luby_value v;
        int ok = eval_ok(L, "param and local both scoped",
            "def f(a)\n"
            "  b = a * 2\n"
            "  b\n"
            "end\n"
            "a = 100\n"
            "b = 200\n"
            "f(5)\n"
            "a + b\n", &v)
          && assert_int("a+b == 300", v, 300);
        test("param and local both scoped", ok);
        luby_free(L);
    }

    /* ---- Bug #3: Can't shadow builtins ---- */
    printf("\n-- Builtin Shadowing --\n");

    {
        luby_state *L = new_state();
        luby_value v;
        int ok = eval_ok(L, "shadow puts",
            "def puts(x)\n"
            "  to_s(x) + \"!\"\n"
            "end\n"
            "puts(\"hello\")\n", &v)
          && assert_string("puts(hello) == hello!", v, "hello!");
        test("shadow puts with user function", ok);
        luby_free(L);
    }

    {
        luby_state *L = new_state();
        luby_value v;
        int ok = eval_ok(L, "shadow p",
            "def p(x)\n"
            "  x * 2\n"
            "end\n"
            "p(21)\n", &v)
          && assert_int("p(21) == 42", v, 42);
        test("shadow p with user function", ok);
        luby_free(L);
    }

    {
        luby_state *L = new_state();
        luby_value v;
        int ok = eval_ok(L, "shadow dot",
            "def dot(a, b)\n"
            "  sum = 0\n"
            "  i = 0\n"
            "  while i < len(a)\n"
            "    sum = sum + a[i] * b[i]\n"
            "    i = i + 1\n"
            "  end\n"
            "  sum\n"
            "end\n"
            "dot([1, 3, -5], [4, -2, -1])\n", &v)
          && assert_int("dot product == 3", v, 3);
        test("shadow dot (the original bug)", ok);
        luby_free(L);
    }

    {
        luby_state *L = new_state();
        luby_value v;
        int ok = eval_ok(L, "shadow max",
            "def max(a, b)\n"
            "  if a > b\n"
            "    a\n"
            "  else\n"
            "    b\n"
            "  end\n"
            "end\n"
            "max(3, 7)\n", &v)
          && assert_int("max(3,7) == 7", v, 7);
        test("shadow max with user function", ok);
        luby_free(L);
    }

    /* ---- Combined: scoping + shadowing ---- */
    printf("\n-- Combined --\n");

    {
        luby_state *L = new_state();
        luby_value v;
        int ok = eval_ok(L, "shadowed fn with proper scoping",
            "def dot(a, b)\n"
            "  sum = 0\n"
            "  i = 0\n"
            "  while i < len(a)\n"
            "    sum = sum + a[i] * b[i]\n"
            "    i = i + 1\n"
            "  end\n"
            "  sum\n"
            "end\n"
            "sum = 999\n"
            "r = dot([1, 3, -5], [4, -2, -1])\n"
            "sum\n", &v)
          && assert_int("caller sum == 999", v, 999);
        test("shadowed fn with proper scoping", ok);
        luby_free(L);
    }

    {
        luby_state *L = new_state();
        luby_value v;
        int ok = eval_ok(L, "multiple calls with scoped locals",
            "def add_squares(a, b)\n"
            "  sa = a * a\n"
            "  sb = b * b\n"
            "  sa + sb\n"
            "end\n"
            "r1 = add_squares(3, 4)\n"
            "r2 = add_squares(5, 12)\n"
            "to_s(r1) + \",\" + to_s(r2)\n", &v)
          && assert_string("results", v, "25,169");
        test("multiple calls with scoped locals", ok);
        luby_free(L);
    }

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
