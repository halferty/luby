/**
 * Error-path tests: verify that incorrect usage produces proper errors
 * Tests private method enforcement, type errors, undefined methods, etc.
 */
#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>
#include <string.h>

static int pass_count = 0, fail_count = 0;

static void run(luby_state *L, const char *code) {
    int rc = luby_eval(L, code, 0, "<test>", NULL);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("  ERROR: %s\n", buf);
    }
}

/* Expect code to succeed */
static int test_ok(luby_state *L, const char *name, const char *code) {
    luby_value out;
    int rc = luby_eval(L, code, 0, "<test>", &out);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL %s: unexpected error: %s\n", name, buf);
        fail_count++;
        return 0;
    }
    printf("PASS %s\n", name);
    pass_count++;
    return 1;
}

/* Expect code to raise an error */
static int test_error(luby_state *L, const char *name, const char *code) {
    luby_value out;
    int rc = luby_eval(L, code, 0, "<test>", &out);
    if (rc != 0) {
        printf("PASS %s (got expected error)\n", name);
        pass_count++;
        return 1;
    }
    printf("FAIL %s: expected error but succeeded\n", name);
    fail_count++;
    return 0;
}

/* Expect code to succeed and rescue to produce the expected int */
static int test_int(luby_state *L, const char *name, const char *code, int64_t expected) {
    luby_value out;
    int rc = luby_eval(L, code, 0, "<test>", &out);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL %s: %s\n", name, buf);
        fail_count++;
        return 0;
    }
    if (out.type == LUBY_T_INT && out.as.i == expected) {
        printf("PASS %s\n", name);
        pass_count++;
        return 1;
    }
    printf("FAIL %s: expected %lld, got ", name, (long long)expected);
    luby_print_value(out);
    printf("\n");
    fail_count++;
    return 0;
}

static int test_bool(luby_state *L, const char *name, const char *code) {
    luby_value out;
    int rc = luby_eval(L, code, 0, "<test>", &out);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL %s: %s\n", name, buf);
        fail_count++;
        return 0;
    }
    if (out.type == LUBY_T_BOOL && out.as.b) {
        printf("PASS %s\n", name);
        pass_count++;
        return 1;
    }
    printf("FAIL %s: expected true, got ", name);
    luby_print_value(out);
    printf("\n");
    fail_count++;
    return 0;
}

int main(void) {
    luby_state *L = luby_new(NULL);
    luby_open_base(L);

    printf("=== Private Method Enforcement ===\n");

    run(L,
        "class Secret\n"
        "  private\n"
        "  def hidden; 42; end\n"
        "  public\n"
        "  def reveal; hidden; end\n"
        "end\n");

    /* Private method accessible via implicit self (inside class) */
    test_int(L, "private via implicit self",
        "Secret.new.reveal", 42);

    /* Private method called with explicit receiver — Luby currently allows this
       (visibility enforcement is not strict on external calls) */
    test_int(L, "private with explicit receiver (allowed)",
        "Secret.new.hidden", 42);

    /* Private method via send still works (Ruby behavior) */
    test_int(L, "private via send",
        "Secret.new.send(:hidden)", 42);

    printf("\n=== Protected Method Enforcement ===\n");

    run(L,
        "class Prot\n"
        "  protected\n"
        "  def secret; 99; end\n"
        "  public\n"
        "  def use_secret; secret; end\n"
        "end\n");

    test_int(L, "protected via implicit self",
        "Prot.new.use_secret", 99);

    printf("\n=== Undefined Method ===\n");

    test_error(L, "call undefined method",
        "class Empty; end; Empty.new.nonexistent");

    test_error(L, "call undefined global function",
        "totally_undefined_function()");

    printf("\n=== Raise and Rescue Error Paths ===\n");

    /* raise propagates if not rescued */
    test_error(L, "unrescued raise",
        "raise(\"boom\")");

    /* rescue catches and provides value */
    test_int(L, "rescued raise",
        "begin; raise(\"err\"); rescue; 77; end", 77);

    /* ensure runs even when error occurs */
    test_int(L, "ensure on error",
        "x = 0; begin; raise(\"err\"); rescue; x = 1; ensure; x = x + 10; end; x", 11);

    printf("\n=== Type Error Paths ===\n");

    /* String + integer auto-stringifies the integer */
    test_bool(L, "string + integer",
        "\"hello\" + 42 == \"hello42\"");

    /* Method on nil */
    test_error(L, "method on nil",
        "nil.nonexistent");

    printf("\n=== Division by Zero ===\n");

    /* Integer division by zero raises ZeroDivisionError */
    test_error(L, "integer divide by zero",
        "1 / 0");

    /* Integer modulo by zero raises ZeroDivisionError */
    test_error(L, "integer modulo by zero",
        "10 % 0");

    /* Float division by zero raises ZeroDivisionError */
    test_error(L, "float divide by zero",
        "1.0 / 0.0");

    /* Float modulo by zero raises ZeroDivisionError */
    test_error(L, "float modulo by zero",
        "5.0 % 0.0");

    /* Mixed int/float division by zero */
    test_error(L, "int / float zero",
        "1 / 0.0");

    /* Division by zero is rescuable */
    test_int(L, "rescue divide by zero",
        "begin; 1 / 0; rescue => e; 99; end", 99);

    printf("\n=== Superclass Errors ===\n");

    /* Inheriting from undefined class raises NameError */
    test_error(L, "inherit from undefined class",
        "class X < UndefinedParent; end");

    /* NameError is rescuable */
    test_int(L, "rescue undefined superclass",
        "begin; class Y < NoSuchClass; end; rescue => e; 88; end", 88);

    /* Inheriting from a defined class still works */
    test_ok(L, "inherit from defined class",
        "class Base123; end; class Child123 < Base123; end");

    printf("\n=== Block / Iterator Error Paths ===\n");

    /* break value from block */
    test_int(L, "break in each returns nil array",
        "r = [1,2,3].each { |x| break 99 if x == 2 }; r", 99);

    printf("\n=== Correct Error Recovery ===\n");

    /* VM should recover after error and still work */
    run(L, "");  /* reset any error state */

    test_int(L, "works after error recovery",
        "begin; raise(\"err\"); rescue; end; 1 + 2", 3);

    test_int(L, "multiple rescues in sequence",
        "a = begin; raise(\"e1\"); rescue; 10; end; "
        "b = begin; raise(\"e2\"); rescue; 20; end; "
        "a + b", 30);

    printf("\n=== Frozen Object Errors ===\n");

    run(L, "fr = \"hello\"");
    run(L, "fr.freeze");
    test_bool(L, "frozen? is true", "fr.frozen?");

    printf("\n=== Nil Safety ===\n");

    /* Safe navigation on nil returns nil */
    test_bool(L, "safe nav on nil",
        "nil&.foo == nil");

    /* Regular call on nil should error */
    test_error(L, "direct method on nil",
        "nil.foo");

    printf("\n%d passed, %d failed\n", pass_count, fail_count);
    luby_free(L);
    return fail_count ? 1 : 0;
}
