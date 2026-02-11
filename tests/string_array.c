/**
 * Tests for interpreter deficiencies found during Project Euler work:
 *   1. String indexing:  s[i]  should return a one-character string
 *   2. String reverse:   reverse(s)  should return the reversed string
 *   3. Array + operator: [1,2] + [3,4]  should concatenate arrays
 */
#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* helpers                                                             */
/* ------------------------------------------------------------------ */

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

static int eval_check(luby_state *L, const char *label, const char *code, luby_value *out) {
    int rc = luby_eval(L, code, 0, "<test>", out);
    if (rc != 0) {
        char buf[512];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL: %s — runtime error: %s\n", label, buf);
        return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(void) {
    int ok = 1;
    luby_state *L = luby_new(NULL);
    if (!L) { printf("FAIL: luby_new\n"); return 1; }
    luby_open_base(L);

    luby_value out;

    /* ============================================================== */
    /*  1. String indexing: s[i]                                      */
    /* ============================================================== */
    printf("--- String indexing ---\n");

    /* Basic: first character */
    if (eval_check(L, "str[0]", "\"hello\"[0]", &out)) {
        ok &= assert_string("str[0]", out, "h");
    } else { ok = 0; }

    /* Middle character */
    if (eval_check(L, "str[2]", "\"hello\"[2]", &out)) {
        ok &= assert_string("str[2]", out, "l");
    } else { ok = 0; }

    /* Last character */
    if (eval_check(L, "str[4]", "\"hello\"[4]", &out)) {
        ok &= assert_string("str[4]", out, "o");
    } else { ok = 0; }

    /* Out of bounds → nil */
    if (eval_check(L, "str[99]", "\"hi\"[99]", &out)) {
        if (out.type != LUBY_T_NIL) {
            printf("FAIL: str[99] (expected nil, got type=%d)\n", out.type);
            ok = 0;
        }
    } else { ok = 0; }

    /* Index with variable */
    if (eval_check(L, "str[var]", "s = \"abcde\"; i = 3; s[i]", &out)) {
        ok &= assert_string("str[var]", out, "d");
    } else { ok = 0; }

    /* Using string index in arithmetic via to_i */
    if (eval_check(L, "digit char to_i", "to_i(\"9375\"[1])", &out)) {
        ok &= assert_int("digit char to_i", out, 3);
    } else { ok = 0; }

    /* Iterate over a string's characters */
    if (eval_check(L, "str index loop",
        "s = \"abc\"\n"
        "result = \"\"\n"
        "i = 0\n"
        "while i < len(s)\n"
        "  result = result + s[i]\n"
        "  i = i + 1\n"
        "end\n"
        "result\n", &out)) {
        ok &= assert_string("str index loop", out, "abc");
    } else { ok = 0; }

    /* ============================================================== */
    /*  2. String reverse: reverse(s)                                 */
    /* ============================================================== */
    printf("--- String reverse ---\n");

    if (eval_check(L, "reverse str", "reverse(\"hello\")", &out)) {
        ok &= assert_string("reverse str", out, "olleh");
    } else { ok = 0; }

    if (eval_check(L, "reverse empty", "reverse(\"\")", &out)) {
        ok &= assert_string("reverse empty", out, "");
    } else { ok = 0; }

    if (eval_check(L, "reverse single", "reverse(\"x\")", &out)) {
        ok &= assert_string("reverse single", out, "x");
    } else { ok = 0; }

    if (eval_check(L, "reverse palindrome", "reverse(\"racecar\")", &out)) {
        ok &= assert_string("reverse palindrome", out, "racecar");
    } else { ok = 0; }

    /* Array reverse should still work */
    if (eval_check(L, "reverse arr still works", "a = reverse([1,2,3]); a[0]", &out)) {
        ok &= assert_int("reverse arr still works", out, 3);
    } else { ok = 0; }

    /* ============================================================== */
    /*  3. Array + operator                                           */
    /* ============================================================== */
    printf("--- Array + operator ---\n");

    if (eval_check(L, "arr + arr len", "a = [1,2] + [3,4]; len(a)", &out)) {
        ok &= assert_int("arr + arr len", out, 4);
    } else { ok = 0; }

    if (eval_check(L, "arr + arr values", "a = [10,20] + [30]; a[0] + a[1] + a[2]", &out)) {
        ok &= assert_int("arr + arr values", out, 60);
    } else { ok = 0; }

    if (eval_check(L, "arr + empty", "a = [1,2] + []; len(a)", &out)) {
        ok &= assert_int("arr + empty", out, 2);
    } else { ok = 0; }

    if (eval_check(L, "empty + arr", "a = [] + [5,6]; a[0]", &out)) {
        ok &= assert_int("empty + arr", out, 5);
    } else { ok = 0; }

    /* Chained concat */
    if (eval_check(L, "arr + arr + arr", "a = [1] + [2] + [3]; a[0] + a[1] + a[2]", &out)) {
        ok &= assert_int("arr + arr + arr", out, 6);
    } else { ok = 0; }

    /* Original arrays should be unchanged (non-mutating) */
    if (eval_check(L, "arr + non-mutating", "a = [1,2]; b = [3,4]; c = a + b; len(a) + len(b)", &out)) {
        ok &= assert_int("arr + non-mutating", out, 4);
    } else { ok = 0; }

    /* Integer + should still work */
    if (eval_check(L, "int + still works", "3 + 4", &out)) {
        ok &= assert_int("int + still works", out, 7);
    } else { ok = 0; }

    /* String + should still work */
    if (eval_check(L, "str + still works", "\"ab\" + \"cd\"", &out)) {
        ok &= assert_string("str + still works", out, "abcd");
    } else { ok = 0; }

    /* ============================================================== */
    /*  Euler 4 with string palindrome (was our workaround)           */
    /* ============================================================== */
    printf("--- Euler 4 string palindrome ---\n");

    if (eval_check(L, "euler4 str palindrome",
        "def is_palindrome(n)\n"
        "  s = to_s(n)\n"
        "  s == reverse(s)\n"
        "end\n"
        "\n"
        "best = 0\n"
        "i = 999\n"
        "while i >= 100\n"
        "  j = i\n"
        "  while j >= 100\n"
        "    p = i * j\n"
        "    if p > best && is_palindrome(p)\n"
        "      best = p\n"
        "    end\n"
        "    j = j - 1\n"
        "  end\n"
        "  i = i - 1\n"
        "end\n"
        "best\n", &out)) {
        ok &= assert_int("euler4 str palindrome", out, 906609);
    } else { ok = 0; }

    /* ============================================================== */

    luby_free(L);

    printf("\n");
    if (ok) {
        printf("All string_array tests PASSED\n");
    } else {
        printf("Some string_array tests FAILED\n");
    }
    return ok ? 0 : 1;
}
