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

int main(void) {
    luby_state *L = luby_new(NULL);
    luby_open_base(L);

    printf("=== Lazy Enumerator Tests ===\n\n");

    /* ---- Basic .lazy and .to_a ---- */
    printf("--- Basic lazy/to_a ---\n");

    test_bool(L, "array_lazy_to_a",
        "r = [1,2,3].lazy.to_a; r[0] == 1 && r[1] == 2 && r[2] == 3");

    test_int(L, "array_lazy_to_a_len",
        "length([1,2,3,4,5].lazy.to_a)", 5);

    test_bool(L, "range_lazy_to_a",
        "r = (1..5).lazy.to_a; r[0] == 1 && r[4] == 5 && length(r) == 5");

    /* ---- map ---- */
    printf("\n--- map ---\n");

    test_bool(L, "lazy_map",
        "r = [1,2,3].lazy.map { |x| x * 2 }.to_a; "
        "r[0] == 2 && r[1] == 4 && r[2] == 6");

    test_bool(L, "lazy_map_range",
        "r = (1..4).lazy.map { |x| x * 10 }.to_a; "
        "r[0] == 10 && r[3] == 40 && length(r) == 4");

    /* ---- select / filter ---- */
    printf("\n--- select / filter ---\n");

    test_bool(L, "lazy_select",
        "r = [1,2,3,4,5,6].lazy.select { |x| x > 3 }.to_a; "
        "r[0] == 4 && r[1] == 5 && r[2] == 6 && length(r) == 3");

    test_bool(L, "lazy_select_range",
        "r = (1..10).lazy.select { |x| x % 2 == 0 }.to_a; "
        "r[0] == 2 && r[4] == 10 && length(r) == 5");

    /* ---- reject ---- */
    printf("\n--- reject ---\n");

    test_bool(L, "lazy_reject",
        "r = [1,2,3,4,5].lazy.reject { |x| x % 2 == 0 }.to_a; "
        "r[0] == 1 && r[1] == 3 && r[2] == 5 && length(r) == 3");

    /* ---- take ---- */
    printf("\n--- take ---\n");

    test_int(L, "lazy_take_len",
        "length([1,2,3,4,5].lazy.take(3).to_a)", 3);

    test_bool(L, "lazy_take_values",
        "r = [10,20,30,40,50].lazy.take(2).to_a; r[0] == 10 && r[1] == 20");

    test_int(L, "lazy_take_range",
        "length((1..1000).lazy.take(5).to_a)", 5);

    test_bool(L, "lazy_take_range_values",
        "r = (1..1000).lazy.take(3).to_a; r[0] == 1 && r[1] == 2 && r[2] == 3");

    /* ---- drop ---- */
    printf("\n--- drop ---\n");

    test_bool(L, "lazy_drop",
        "r = [1,2,3,4,5].lazy.drop(2).to_a; "
        "r[0] == 3 && r[1] == 4 && r[2] == 5 && length(r) == 3");

    test_bool(L, "lazy_drop_range",
        "r = (1..5).lazy.drop(3).to_a; r[0] == 4 && r[1] == 5 && length(r) == 2");

    /* ---- flat_map ---- */
    printf("\n--- flat_map ---\n");

    test_bool(L, "lazy_flat_map",
        "r = [1,2,3].lazy.flat_map { |x| [x, x * 10] }.to_a; "
        "r[0] == 1 && r[1] == 10 && r[2] == 2 && r[3] == 20 && r[4] == 3 && r[5] == 30");

    /* ---- Chaining ---- */
    printf("\n--- Chaining ---\n");

    test_bool(L, "chain_select_map",
        "r = [1,2,3,4,5,6].lazy.select { |x| x > 2 }.map { |x| x * 10 }.to_a; "
        "r[0] == 30 && r[1] == 40 && r[2] == 50 && r[3] == 60");

    test_bool(L, "chain_map_select",
        "r = [1,2,3,4,5].lazy.map { |x| x * 2 }.select { |x| x > 4 }.to_a; "
        "r[0] == 6 && r[1] == 8 && r[2] == 10");

    test_bool(L, "chain_select_map_take",
        "r = (1..100).lazy.select { |x| x % 3 == 0 }.map { |x| x * x }.take(4).to_a; "
        "r[0] == 9 && r[1] == 36 && r[2] == 81 && r[3] == 144 && length(r) == 4");

    test_bool(L, "chain_map_take",
        "r = (1..1000).lazy.map { |x| x * x }.take(5).to_a; "
        "r[0] == 1 && r[1] == 4 && r[2] == 9 && r[3] == 16 && r[4] == 25");

    test_bool(L, "chain_drop_take",
        "r = (1..100).lazy.drop(5).take(3).to_a; "
        "r[0] == 6 && r[1] == 7 && r[2] == 8");

    /* ---- first ---- */
    printf("\n--- first ---\n");

    test_int(L, "lazy_first_one",
        "[10,20,30].lazy.first", 10);

    test_bool(L, "lazy_first_n",
        "r = [10,20,30,40].lazy.first(2); r[0] == 10 && r[1] == 20");

    test_int(L, "lazy_first_with_chain",
        "(1..1000).lazy.select { |x| x > 50 }.first", 51);

    test_bool(L, "lazy_first_n_with_chain",
        "r = (1..1000).lazy.map { |x| x * 2 }.select { |x| x > 10 }.first(3); "
        "r[0] == 12 && r[1] == 14 && r[2] == 16");

    /* ---- force ---- */
    printf("\n--- force ---\n");

    test_int(L, "lazy_force",
        "length([1,2,3].lazy.select { |x| x > 1 }.force)", 2);

    /* ---- each ---- */
    printf("\n--- each ---\n");

    test_int(L, "lazy_each",
        "sum = 0; [1,2,3,4,5].lazy.select { |x| x > 2 }.each { |x| sum = sum + x }; sum",
        12);

    /* ---- Consuming methods on Lazy ---- */
    printf("\n--- Consuming methods ---\n");

    test_int(L, "lazy_count",
        "(1..20).lazy.select { |x| x % 2 == 0 }.count", 10);

    test_int(L, "lazy_sum",
        "(1..10).lazy.select { |x| x % 2 == 0 }.sum", 30);

    test_int(L, "lazy_min",
        "[5,1,3,2,4].lazy.select { |x| x > 2 }.min", 3);

    test_int(L, "lazy_max",
        "[5,1,3,2,4].lazy.select { |x| x < 4 }.max", 3);

    test_int(L, "lazy_reduce",
        "(1..5).lazy.map { |x| x * 2 }.reduce(0) { |acc, x| acc + x }", 30);

    test_int(L, "lazy_find",
        "(1..100).lazy.map { |x| x * x }.find { |x| x > 50 }", 64);

    test_bool(L, "lazy_any",
        "[1,2,3,4,5].lazy.any? { |x| x > 3 }");

    test_bool(L, "lazy_all",
        "[2,4,6,8].lazy.all? { |x| x % 2 == 0 }");

    test_bool(L, "lazy_none",
        "[1,3,5,7].lazy.none? { |x| x % 2 == 0 }");

    test_bool(L, "lazy_include",
        "(1..100).lazy.map { |x| x * 3 }.include?(21)");

    /* ---- .lazy.lazy returns self ---- */
    printf("\n--- lazy.lazy ---\n");

    test_bool(L, "lazy_lazy",
        "a = [1,2,3].lazy; b = a.lazy; r = b.to_a; r[0] == 1 && r[2] == 3");

    /* ---- Large range with take (performance / short-circuit) ---- */
    printf("\n--- Short-circuit performance ---\n");

    test_bool(L, "large_range_take",
        "r = (1..1000000).lazy.select { |x| x % 1000 == 0 }.take(3).to_a; "
        "r[0] == 1000 && r[1] == 2000 && r[2] == 3000");

    /* ---- Enumerable#lazy on custom class ---- */
    printf("\n--- Enumerable#lazy on custom class ---\n");

    run(L,
        "class Trio\n"
        "  include Enumerable\n"
        "  def initialize(a, b, c)\n"
        "    @a = a\n"
        "    @b = b\n"
        "    @c = c\n"
        "  end\n"
        "  def each(&blk)\n"
        "    blk.call(@a)\n"
        "    blk.call(@b)\n"
        "    blk.call(@c)\n"
        "  end\n"
        "end\n");

    test_bool(L, "custom_enumerable_lazy",
        "r = Trio.new(10, 20, 30).lazy.map { |x| x + 1 }.to_a; "
        "r[0] == 11 && r[1] == 21 && r[2] == 31");

    printf("\n%d passed, %d failed\n", pass_count, fail_count);
    luby_free(L);
    return fail_count ? 1 : 0;
}
