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

int main(void) {
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    int ok = 1;

    // === map_with_index ===
    ok &= test_int(L, "map_with_index basic",
        "a = [10, 20, 30].map_with_index { |x, i| x + i }; a[0]", 10);
    ok &= test_int(L, "map_with_index second",
        "a = [10, 20, 30].map_with_index { |x, i| x + i }; a[1]", 21);
    ok &= test_int(L, "map_with_index third",
        "a = [10, 20, 30].map_with_index { |x, i| x + i }; a[2]", 32);
    ok &= test_int(L, "map_with_index index only",
        "a = [\"a\", \"b\", \"c\"].map_with_index { |x, i| i }; a[2]", 2);

    // === each_with_object ===
    ok &= test_int(L, "each_with_object returns object",
        "r = [1, 2, 3].each_with_object([]) { |x, memo| memo }; len(r)", 0);

    // === each_slice ===
    ok &= test_int(L, "each_slice count",
        "a = [1, 2, 3, 4, 5].each_slice(2); len(a)", 3);
    ok &= test_int(L, "each_slice first slice",
        "a = [1, 2, 3, 4, 5].each_slice(2); len(a[0])", 2);
    ok &= test_int(L, "each_slice last slice",
        "a = [1, 2, 3, 4, 5].each_slice(2); len(a[2])", 1);
    ok &= test_int(L, "each_slice values",
        "a = [1, 2, 3, 4, 5].each_slice(2); a[0][0] + a[0][1]", 3);
    ok &= test_int(L, "each_slice exact",
        "a = [1, 2, 3, 4].each_slice(2); len(a)", 2);

    // === each_cons ===
    ok &= test_int(L, "each_cons count",
        "a = [1, 2, 3, 4].each_cons(2); len(a)", 3);
    ok &= test_int(L, "each_cons first pair",
        "a = [1, 2, 3, 4].each_cons(2); a[0][0] + a[0][1]", 3);
    ok &= test_int(L, "each_cons last pair",
        "a = [1, 2, 3, 4].each_cons(2); a[2][0] + a[2][1]", 7);
    ok &= test_int(L, "each_cons triplets",
        "a = [1, 2, 3, 4, 5].each_cons(3); len(a)", 3);
    ok &= test_int(L, "each_cons too large",
        "a = [1, 2].each_cons(5); len(a)", 0);

    // === find_index ===
    ok &= test_int(L, "find_index with block",
        "[10, 20, 30, 40].find_index { |x| x > 25 }", 2);
    ok &= test_int(L, "find_index first match",
        "[1, 2, 3, 2, 1].find_index { |x| x == 2 }", 1);
    ok &= test_nil(L, "find_index no match",
        "[1, 2, 3].find_index { |x| x > 10 }");
    ok &= test_int(L, "find_index with value",
        "[10, 20, 30].find_index(20)", 1);
    ok &= test_nil(L, "find_index value not found",
        "[10, 20, 30].find_index(99)");
    ok &= test_int(L, "index alias",
        "[10, 20, 30].index(30)", 2);

    // === tally ===
    ok &= test_int(L, "tally counts",
        "h = [1, 2, 1, 3, 2, 1].tally; h[1]", 3);
    ok &= test_int(L, "tally single",
        "h = [1, 2, 1, 3, 2, 1].tally; h[3]", 1);
    ok &= test_int(L, "tally two",
        "h = [1, 2, 1, 3, 2, 1].tally; h[2]", 2);

    luby_free(L);
    printf("\nwith_index tests: %s\n", ok ? "ALL PASSED" : "SOME FAILED");
    return ok ? 0 : 1;
}
