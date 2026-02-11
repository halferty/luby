#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>

static int test(luby_state *L, const char *name, const char *code, int expected) {
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

int main(void) {
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    int ok = 1;
    
    // unless
    ok &= test(L, "unless false", "x = 0; unless false; x = 1; end; x", 1);
    ok &= test(L, "unless true", "x = 0; unless true; x = 1; end; x", 0);
    
    // until
    ok &= test(L, "until", "x = 0; until x >= 3; x = x + 1; end; x", 3);
    
    // case/when
    ok &= test(L, "case/when", "x = 2; case x; when 1; 10; when 2; 20; else; 30; end", 20);
    
    // super
    ok &= test(L, "super", "class A; def x; 10; end; end; class B < A; def x; super + 5; end; end; B.new.x", 15);
    
    // ranges
    ok &= test(L, "range inclusive", "r = 1..3; sum = 0; each(r) { |i| sum = sum + i }; sum", 6);
    
    // ternary
    ok &= test(L, "ternary true", "true ? 1 : 2", 1);
    ok &= test(L, "ternary false", "false ? 1 : 2", 2);
    
    // instance variables
    ok &= test(L, "ivar", "class P; def set(v); @x = v; end; def get; @x; end; end; p = P.new; p.set(42); p.get", 42);
    
    luby_free(L);
    return ok ? 0 : 1;
}
