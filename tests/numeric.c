/**
 * Test numeric predicates
 */
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
    
    // zero? tests
    test(L, "zero? on 0", "0.zero?");
    test(L, "zero? on non-zero", "!5.zero?");
    test(L, "zero? on negative", "!(-1).zero?");
    test(L, "zero? on float 0.0", "0.0.zero?");
    test(L, "zero? on float non-zero", "!3.14.zero?");
    
    // positive? tests
    test(L, "positive? on positive", "5.positive?");
    test(L, "positive? on zero", "!0.positive?");
    test(L, "positive? on negative", "!(-5).positive?");
    test(L, "positive? on float", "3.14.positive?");
    test(L, "positive? on negative float", "!(-2.5).positive?");
    
    // negative? tests
    test(L, "negative? on negative", "(-5).negative?");
    test(L, "negative? on zero", "!0.negative?");
    test(L, "negative? on positive", "!5.negative?");
    test(L, "negative? on float", "(-3.14).negative?");
    test(L, "negative? on positive float", "!2.5.negative?");
    
    // even? tests (already existed)
    test(L, "even? on even", "4.even?");
    test(L, "even? on odd", "!5.even?");
    test(L, "even? on zero", "0.even?");
    test(L, "even? on negative even", "(-2).even?");
    
    // odd? tests (already existed)
    test(L, "odd? on odd", "5.odd?");
    test(L, "odd? on even", "!4.odd?");
    test(L, "odd? on negative odd", "(-3).odd?");
    
    // abs tests (already existed)
    test(L, "abs on positive", "abs(5) == 5");
    test(L, "abs on negative", "abs(-5) == 5");
    test(L, "abs on zero", "abs(0) == 0");
    test(L, "abs on float", "abs(-3.5) == 3.5");
    
    // ceil, floor, round tests (already existed)
    test(L, "ceil on positive", "ceil(3.2) == 4");
    test(L, "ceil on negative", "ceil(-3.2) == -3");
    test(L, "floor on positive", "floor(3.8) == 3");
    test(L, "floor on negative", "floor(-3.8) == -4");
    test(L, "round on .5", "round(3.5) == 4");
    test(L, "round on .4", "round(3.4) == 3");
    
    // Combined tests
    test(L, "chained predicates", 
         "x = 5; x.positive? && x.odd? && !x.zero?");
    
    test(L, "negative zero check",
         "x = -10; x.negative? && x.even? && !x.zero?");
    
    test(L, "zero with operations",
         "x = 5 - 5; x.zero? && !x.positive? && !x.negative?");
    
    luby_free(L);
    return 0;
}
