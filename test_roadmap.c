#define LUBY_IMPLEMENTATION
#include "luby.h"
#include <stdio.h>
#include <stdlib.h>

#define TEST(name) printf("\n=== Test: %s ===\n", name)
#define CHECK(expr) do { \
    if (!(expr)) { \
        printf("FAILED: %s at line %d\n", #expr, __LINE__); \
        return 1; \
    } \
} while (0)

int main(void) {
    luby_state *L = luby_new(NULL);
    if (!L) {
        printf("Failed to create Luby state\n");
        return 1;
    }
    luby_open_base(L);

    luby_value result;
    int rc;

    // Test 1: Object#inspect for strings
    TEST("inspect on string");
    rc = luby_eval(L, "\"hello\\nworld\".inspect", 0, "<test>", &result);
    CHECK(rc == 0);
    CHECK(result.type == LUBY_T_STRING);
    printf("Result: %s\n", (char*)result.as.ptr);

    // Test 2: Object#inspect for symbols
    TEST("inspect on symbol");
    rc = luby_eval(L, ":test_symbol.inspect", 0, "<test>", &result);
    CHECK(rc == 0);
    CHECK(result.type == LUBY_T_STRING);
    printf("Result: %s\n", (char*)result.as.ptr);

    // Test 3: Object#inspect for integers
    TEST("inspect on integer");
    rc = luby_eval(L, "42.inspect", 0, "<test>", &result);
    CHECK(rc == 0);
    CHECK(result.type == LUBY_T_STRING);
    printf("Result: %s\n", (char*)result.as.ptr);

    // Test 4: Object#inspect for nil
    TEST("inspect on nil");
    rc = luby_eval(L, "nil.inspect", 0, "<test>", &result);
    CHECK(rc == 0);
    CHECK(result.type == LUBY_T_STRING);
    printf("Result: %s\n", (char*)result.as.ptr);

    // Test 5: Object#object_id for nil
    TEST("object_id on nil");
    rc = luby_eval(L, "nil.object_id", 0, "<test>", &result);
    CHECK(rc == 0);
    CHECK(result.type == LUBY_T_INT);
    CHECK(result.as.i == 4);
    printf("Result: %lld\n", (long long)result.as.i);

    // Test 6: Object#object_id for true/false
    TEST("object_id on true");
    rc = luby_eval(L, "true.object_id", 0, "<test>", &result);
    CHECK(rc == 0);
    CHECK(result.type == LUBY_T_INT);
    CHECK(result.as.i == 20);
    printf("Result: %lld\n", (long long)result.as.i);

    TEST("object_id on false");
    rc = luby_eval(L, "false.object_id", 0, "<test>", &result);
    CHECK(rc == 0);
    CHECK(result.type == LUBY_T_INT);
    CHECK(result.as.i == 0);
    printf("Result: %lld\n", (long long)result.as.i);

    // Test 7: Object#object_id for integers
    TEST("object_id on integer");
    rc = luby_eval(L, "5.object_id", 0, "<test>", &result);
    CHECK(rc == 0);
    CHECK(result.type == LUBY_T_INT);
    CHECK(result.as.i == 11);  // (5 * 2) + 1
    printf("Result: %lld\n", (long long)result.as.i);

    // Test 8: Class#name
    TEST("Class#name");
    rc = luby_eval(L, "class Foo\nend\nFoo.name", 0, "<test>", &result);
    CHECK(rc == 0);
    CHECK(result.type == LUBY_T_STRING);
    printf("Result: %s\n", (char*)result.as.ptr);

    // Test 9: Class#superclass
    TEST("Class#superclass");
    rc = luby_eval(L, "class Bar < Foo\nend\nBar.superclass.name", 0, "<test>", &result);
    CHECK(rc == 0);
    CHECK(result.type == LUBY_T_STRING);
    printf("Result: %s\n", (char*)result.as.ptr);

    // Test 10: Class#ancestors
    TEST("Class#ancestors");
    rc = luby_eval(L, "class Baz < Bar\nend\nBaz.ancestors.map { |c| c.name }", 0, "<test>", &result);
    CHECK(rc == 0);
    CHECK(result.type == LUBY_T_ARRAY);
    printf("Ancestors returned as array\n");

    // Test 11: Symbol#to_sym
    TEST("Symbol#to_sym");
    rc = luby_eval(L, ":mysym.to_sym == :mysym", 0, "<test>", &result);
    CHECK(rc == 0);
    CHECK(result.type == LUBY_T_BOOL);
    CHECK(result.as.b == 1);
    printf("Result: %s\n", result.as.b ? "true" : "false");

    // Test 12: String#to_f
    TEST("String#to_f");
    rc = luby_eval(L, "\"3.14\".to_f", 0, "<test>", &result);
    CHECK(rc == 0);
    CHECK(result.type == LUBY_T_FLOAT);
    printf("Result: %g\n", result.as.f);

    // Test 13: respond_to alias
    TEST("respond_to works");
    rc = luby_eval(L, "5.respond_to(:to_s)", 0, "<test>", &result);
    CHECK(rc == 0);
    CHECK(result.type == LUBY_T_BOOL);
    CHECK(result.as.b == 1);
    printf("Result: %s\n", result.as.b ? "true" : "false");

    printf("\n=== All tests passed! ===\n");
    luby_free(L);
    return 0;
}
