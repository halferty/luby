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


int main(void) {
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    int ok = 1;

    // === map(&:method) ===
    ok &= test_str(L, "map(&:to_s) first",
        "[1, 2, 3].map(&:to_s)[0]", "1");
    ok &= test_str(L, "map(&:to_s) last",
        "[10, 20].map(&:to_s)[1]", "20");
    ok &= test_int(L, "map(&:to_i)",
        "[\"1\", \"2\", \"3\"].map(&:to_i)[2]", 3);

    // === select(&:predicate) ===
    ok &= test_int(L, "select(&:even?) count",
        "[1, 2, 3, 4, 5, 6].select(&:even?).count", 3);
    ok &= test_int(L, "select(&:odd?) first",
        "[1, 2, 3, 4, 5].select(&:odd?)[0]", 1);

    // === map(&:upcase) on strings ===
    ok &= test_str(L, "map(&:upcase)",
        "[\"hello\", \"world\"].map(&:upcase)[0]", "HELLO");
    ok &= test_str(L, "map(&:downcase)",
        "[\"FOO\", \"BAR\"].map(&:downcase)[1]", "bar");

    // === map(&:abs) ===
    ok &= test_int(L, "map(&:abs)",
        "[-1, -2, 3].map(&:abs)[1]", 2);

    // === chaining via intermediate variable ===
    ok &= test_int(L, "chained select then count",
        "a = [1, 2, 3, 4].select(&:even?); a.count", 2);
    ok &= test_str(L, "chained map then map",
        "a = [1, 2, 3].map(&:to_s); b = a.map(&:upcase); b[0]", "1");

    // === reject(&:predicate) ===
    ok &= test_int(L, "reject(&:odd?)",
        "[1, 2, 3, 4, 5].reject(&:odd?)[0]", 2);

    luby_free(L);
    printf("\nsymbol_to_proc tests: %s\n", ok ? "ALL PASSED" : "SOME FAILED");
    return ok ? 0 : 1;
}
