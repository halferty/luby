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
    
    // Statement modifiers
    test(L, "if modifier", "x = 5 if true; x");
    test(L, "unless modifier", "x = 5 unless false; x");
    
    // Multiple assignment
    test(L, "multiple assign", "a, b = 1, 2; a + b");
    test(L, "array destructure", "a, b = [1, 2]; a + b");
    test(L, "swap", "a = 1; b = 2; a, b = b, a; a");
    
    // Symbol hash shorthand
    test(L, "symbol hash", "h = {foo: 1, bar: 2}; h[:foo]");
    
    // More operators
    test(L, "or-assign", "x = nil; x ||= 5; x");
    test(L, "and-assign", "x = 5; x &&= 10; x");
    
    // String methods
    test(L, "str length", "len(\"hello\")");
    test(L, "str include", "include?(\"hello\", \"ell\")");
    test(L, "str capitalize", "capitalize(\"hello\")");
    test(L, "str strip", "strip(\"  hi  \")");
    
    // Array methods
    test(L, "arr include", "include?([1,2,3], 2)");
    test(L, "arr index", "index([1,2,3], 2)");
    test(L, "arr concat", "concat([1,2], [3,4])");
    test(L, "arr take", "take([1,2,3,4], 2)");
    test(L, "arr drop", "drop([1,2,3,4], 2)");
    
    // Numeric methods
    test(L, "abs", "abs(-5)");
    test(L, "floor", "floor(3.7)");
    test(L, "ceil", "ceil(3.2)");
    test(L, "round", "round(3.5)");
    test(L, "even?", "even?(4)");
    test(L, "odd?", "odd?(3)");
    
    // attr_accessor
    test(L, "attr_reader", "class Foo; attr_reader :x; def initialize; @x = 42; end; end; Foo.new.x");
    
    luby_free(L);
    return 0;
}
