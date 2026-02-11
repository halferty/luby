#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>

static int test(luby_state *L, const char *name, const char *code) {
    luby_value out;
    int rc = luby_eval(L, code, 0, "<test>", &out);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL %s: %s\n", name, buf);
        return 0;
    }
    printf("PASS %s\n", name);
    return 1;
}

int main(void) {
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    
    // Test various features - stdlib functions
    test(L, "string upcase", "upcase(\"hello\")");
    test(L, "string downcase", "downcase(\"HELLO\")");
    test(L, "array flatten", "flatten([1, [2, 3]])");
    test(L, "hash keys", "keys({1 => 2, 3 => 4})");
    test(L, "to_s", "to_s(42)");
    test(L, "to_i", "to_i(\"42\")");
    test(L, "nil?", "is_nil(nil)");
    test(L, "times", "sum = 0; times(3) { |i| sum = sum + i }; sum");
    test(L, "upto", "sum = 0; upto(1, 3) { |i| sum = sum + i }; sum");
    test(L, "downto", "sum = 0; downto(3, 1) { |i| sum = sum + i }; sum");
    test(L, "modulo assign", "x = 10; x %= 3; x");
    test(L, "plus assign", "x = 5; x += 3; x");
    test(L, "string split", "split(\"a,b,c\", \",\")");
    test(L, "string join", "join([\"a\", \"b\"], \"-\")");
    test(L, "array reverse", "reverse([1, 2, 3])");
    test(L, "array sort", "sort([3, 1, 2])");
    test(L, "array uniq", "uniq([1, 2, 2, 3])");
    test(L, "array first", "first([1, 2, 3])");
    test(L, "array last", "last([1, 2, 3])");
    test(L, "hash values", "values({1 => 2, 3 => 4})");
    
    // Advanced syntax - may not pass yet
    test(L, "stabby lambda", "f = ->(x) { x * 2 }; f.call(5)");
    test(L, "default params", "def foo(x, y=10); x + y; end; foo(5)");
    test(L, "splat args", "def sum(*args); args.reduce(0) { |a, x| a + x }; end; sum(1,2,3)");
    test(L, "block param", "def foo(&block); block.call; end; foo { 42 }");
    
    luby_free(L);
    return 0;
}
