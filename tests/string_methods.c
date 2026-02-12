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

static int test_bool(luby_state *L, const char *name, const char *code, int expected) {
    luby_value out;
    int rc = luby_eval(L, code, 0, "<test>", &out);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL %s: %s\n", name, buf);
        return 0;
    }
    if (out.type != LUBY_T_BOOL || out.as.b != expected) {
        printf("FAIL %s: expected %s, got ", name, expected ? "true" : "false");
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

    // === chars ===
    ok &= test_int(L, "chars count",
        "\"hello\".chars.count", 5);
    ok &= test_str(L, "chars first",
        "\"hello\".chars[0]", "h");
    ok &= test_str(L, "chars last",
        "\"hello\".chars[4]", "o");
    ok &= test_int(L, "chars empty",
        "\"\".chars.count", 0);

    // === chomp ===
    ok &= test_str(L, "chomp no newline",
        "\"hello\".chomp", "hello");

    // === lstrip ===
    ok &= test_str(L, "lstrip",
        "\"  hello  \".lstrip", "hello  ");

    // === rstrip ===
    ok &= test_str(L, "rstrip",
        "\"  hello  \".rstrip", "  hello");

    // === tr ===
    ok &= test_str(L, "tr basic",
        "\"hello\".tr(\"el\", \"ip\")", "hippo");
    ok &= test_str(L, "tr no match",
        "\"hello\".tr(\"xyz\", \"abc\")", "hello");
    ok &= test_str(L, "tr single char",
        "\"hello\".tr(\"l\", \"r\")", "herro");

    // === center ===
    ok &= test_str(L, "center default pad",
        "\"hi\".center(10)", "    hi    ");
    ok &= test_str(L, "center custom pad",
        "\"hi\".center(10, \"-\")", "----hi----");
    ok &= test_str(L, "center no pad needed",
        "\"hello\".center(3)", "hello");

    // === ljust ===
    ok &= test_str(L, "ljust default",
        "\"hi\".ljust(6)", "hi    ");
    ok &= test_str(L, "ljust custom pad",
        "\"hi\".ljust(6, \"*\")", "hi****");
    ok &= test_str(L, "ljust no pad needed",
        "\"hello\".ljust(3)", "hello");

    // === rjust ===
    ok &= test_str(L, "rjust default",
        "\"hi\".rjust(6)", "    hi");
    ok &= test_str(L, "rjust custom pad",
        "\"hi\".rjust(6, \"0\")", "0000hi");

    // === include? ===
    ok &= test_bool(L, "include? true",
        "\"hello world\".include?(\"world\")", 1);
    ok &= test_bool(L, "include? false",
        "\"hello world\".include?(\"xyz\")", 0);
    ok &= test_bool(L, "include? partial",
        "\"hello\".include?(\"ell\")", 1);

    // === split (already existed, verify) ===
    ok &= test_int(L, "split count",
        "\"a,b,c\".split(\",\").count", 3);
    ok &= test_str(L, "split first",
        "\"a,b,c\".split(\",\")[0]", "a");

    // === join (already existed, verify) ===
    ok &= test_str(L, "join with sep",
        "[\"a\", \"b\", \"c\"].join(\"-\")", "a-b-c");
    ok &= test_str(L, "join no sep",
        "[\"a\", \"b\", \"c\"].join()", "abc");

    luby_free(L);
    printf("\nstring_methods tests: %s\n", ok ? "ALL PASSED" : "SOME FAILED");
    return ok ? 0 : 1;
}
