#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>
#include <string.h>

static int pass_count = 0, fail_count = 0;

static int eval_check(luby_state *L, const char *label, const char *code, luby_value *out) {
    int rc = luby_eval(L, code, 0, "<test>", out);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL %s: %s\n", label, buf);
        fail_count++;
        return 0;
    }
    return 1;
}

static void run(luby_state *L, const char *code) {
    int rc = luby_eval(L, code, 0, "<test>", NULL);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("  ERROR: %s\n", buf);
    }
}

static int test_bool(luby_state *L, const char *name, const char *code) {
    luby_value out;
    if (!eval_check(L, name, code, &out)) return 0;
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

static int test_int(luby_state *L, const char *name, const char *code, int64_t expected) {
    luby_value out;
    if (!eval_check(L, name, code, &out)) return 0;
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

static int test_nil(luby_state *L, const char *name, const char *code) {
    luby_value out;
    if (!eval_check(L, name, code, &out)) return 0;
    if (out.type == LUBY_T_NIL) {
        printf("PASS %s\n", name);
        pass_count++;
        return 1;
    }
    printf("FAIL %s: expected nil, got ", name);
    luby_print_value(out);
    printf("\n");
    fail_count++;
    return 0;
}

static int test_string(luby_state *L, const char *name, const char *code, const char *expected) {
    luby_value out;
    if (!eval_check(L, name, code, &out)) return 0;
    if (out.type == LUBY_T_STRING && out.as.ptr && strcmp((const char *)out.as.ptr, expected) == 0) {
        printf("PASS %s\n", name);
        pass_count++;
        return 1;
    }
    printf("FAIL %s: expected \"%s\", got ", name, expected);
    luby_print_value(out);
    printf("\n");
    fail_count++;
    return 0;
}

int main(void) {
    luby_state *L = luby_new(NULL);
    luby_open_base(L);

    printf("=== Fiber Tests ===\n\n");

    /* ---- Basic creation and resume ---- */
    printf("--- Basic creation and resume ---\n");

    test_int(L, "basic_resume",
        "f = Fiber.new { 42 }; f.resume",
        42);

    test_int(L, "resume_returns_block_result",
        "f = Fiber.new { 10 + 20 }; f.resume",
        30);

    /* ---- Yield and resume ---- */
    printf("\n--- Yield and resume ---\n");

    test_int(L, "yield_value",
        "f = Fiber.new { Fiber.yield(5); 10 }; f.resume",
        5);

    test_int(L, "resume_after_yield",
        "f = Fiber.new { Fiber.yield(5); 10 }; f.resume; f.resume",
        10);

    test_int(L, "multiple_yields",
        "f = Fiber.new { Fiber.yield(1); Fiber.yield(2); 3 }; "
        "a = f.resume; b = f.resume; c = f.resume; "
        "a + b * 10 + c * 100",
        321);

    /* ---- Bidirectional value passing ---- */
    printf("\n--- Bidirectional value passing ---\n");

    test_int(L, "resume_sends_initial_arg",
        "f = Fiber.new { |x| x * 2 }; f.resume(21)",
        42);

    test_int(L, "resume_value_from_yield",
        "f = Fiber.new { |x| v = Fiber.yield(x + 1); v * 2 }; "
        "f.resume(10); f.resume(20)",
        40);

    test_int(L, "bidirectional_chain",
        "f = Fiber.new { |x| "
        "  a = Fiber.yield(x + 1); "
        "  b = Fiber.yield(a + 1); "
        "  b + 1 "
        "}; "
        "r1 = f.resume(10); "
        "r2 = f.resume(r1 + 10); "
        "r3 = f.resume(r2 + 10); "
        "r3",
        33);

    /* ---- Yield with no value ---- */
    printf("\n--- Yield with no value ---\n");

    test_nil(L, "yield_nil_default",
        "f = Fiber.new { Fiber.yield; 99 }; f.resume");

    test_int(L, "resume_after_nil_yield",
        "f = Fiber.new { Fiber.yield; 99 }; f.resume; f.resume",
        99);

    /* ---- alive? ---- */
    printf("\n--- alive? ---\n");

    test_bool(L, "alive_before_resume",
        "f = Fiber.new { Fiber.yield(1); 2 }; f.alive?");

    test_bool(L, "alive_after_yield",
        "f = Fiber.new { Fiber.yield(1); 2 }; f.resume; f.alive?");

    test_bool(L, "dead_after_return",
        "f = Fiber.new { Fiber.yield(1); 2 }; f.resume; f.resume; f.alive? == false");

    /* ---- Dead fiber resume ---- */
    printf("\n--- Dead fiber resume ---\n");

    test_nil(L, "dead_fiber_resume",
        "f = Fiber.new { 42 }; f.resume; f.resume");

    /* ---- Using yield keyword (via OP_YIELD) ---- */
    printf("\n--- yield keyword in fiber ---\n");

    test_int(L, "yield_keyword",
        "f = Fiber.new { yield 5; 10 }; f.resume",
        5);

    test_int(L, "yield_keyword_resume",
        "f = Fiber.new { yield 5; 10 }; f.resume; f.resume",
        10);

    /* ---- Multiple fibers ---- */
    printf("\n--- Multiple fibers ---\n");

    test_int(L, "two_fibers",
        "f1 = Fiber.new { Fiber.yield(1); 2 }; "
        "f2 = Fiber.new { Fiber.yield(10); 20 }; "
        "a = f1.resume; b = f2.resume; c = f1.resume; d = f2.resume; "
        "a + b + c + d",
        33);

    test_int(L, "interleaved_fibers",
        "f1 = Fiber.new { Fiber.yield(1); Fiber.yield(2); 3 }; "
        "f2 = Fiber.new { Fiber.yield(10); Fiber.yield(20); 30 }; "
        "r = 0; "
        "r = r + f1.resume; r = r + f2.resume; "
        "r = r + f1.resume; r = r + f2.resume; "
        "r = r + f1.resume; r = r + f2.resume; "
        "r",
        66);

    /* ---- Fibonacci generator ---- */
    printf("\n--- Fibonacci generator ---\n");

    run(L,
        "fib = Fiber.new {\n"
        "  a = 0\n"
        "  b = 1\n"
        "  loop do\n"
        "    Fiber.yield(a)\n"
        "    c = a + b\n"
        "    a = b\n"
        "    b = c\n"
        "  end\n"
        "}\n");

    test_int(L, "fib_0", "fib.resume", 0);
    test_int(L, "fib_1", "fib.resume", 1);
    test_int(L, "fib_2", "fib.resume", 1);
    test_int(L, "fib_3", "fib.resume", 2);
    test_int(L, "fib_4", "fib.resume", 3);
    test_int(L, "fib_5", "fib.resume", 5);
    test_int(L, "fib_6", "fib.resume", 8);
    test_int(L, "fib_7", "fib.resume", 13);

    /* ---- Producer-consumer ---- */
    printf("\n--- Producer-consumer ---\n");

    test_int(L, "producer_consumer",
        "producer = Fiber.new {\n"
        "  Fiber.yield(1)\n"
        "  Fiber.yield(2)\n"
        "  3\n"
        "}\n"
        "sum = 0\n"
        "while producer.alive?\n"
        "  sum = sum + producer.resume\n"
        "end\n"
        "sum\n",
        6);

    /* ---- Fiber with complex control flow ---- */
    printf("\n--- Complex control flow ---\n");

    test_int(L, "yield_in_loop",
        "f = Fiber.new {\n"
        "  i = 0\n"
        "  while i < 5\n"
        "    Fiber.yield(i)\n"
        "    i = i + 1\n"
        "  end\n"
        "  99\n"
        "}\n"
        "sum = 0\n"
        "6.times { sum = sum + f.resume }\n"
        "sum",
        109);

    /* ---- fiber_new global function (low-level) ---- */
    printf("\n--- Low-level fiber_new ---\n");

    test_int(L, "fiber_new_direct",
        "f = fiber_new { |x| x }; fiber_resume(f, 7)",
        7);

    printf("\n%d passed, %d failed\n", pass_count, fail_count);
    luby_free(L);
    return fail_count ? 1 : 0;
}
