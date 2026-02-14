#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>

static int pass_count = 0, fail_count = 0;

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

static void run(luby_state *L, const char *code) {
    int rc = luby_eval(L, code, 0, "<test>", NULL);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("  ERROR: %s\n", buf);
    }
}

int main(void) {
    luby_state *L = luby_new(NULL);
    luby_open_base(L);

    printf("=== Struct Tests ===\n");

    /* Create struct class */
    run(L, "point = Struct.new(:x, :y)");
    run(L, "p1 = point.new(10, 20)");

    /* Basic accessors */
    test_bool(L, "reader_x", "p1.x == 10");
    test_bool(L, "reader_y", "p1.y == 20");

    /* to_a */
    test_bool(L, "to_a", "r = p1.to_a; r[0] == 10 && r[1] == 20");

    /* members */
    test_bool(L, "members", "r = p1.members; r[0] == :x && r[1] == :y");

    /* == */
    run(L, "p2 = point.new(10, 20)");
    run(L, "p3 = point.new(10, 30)");
    test_bool(L, "eq_same", "p1 == p2");
    test_bool(L, "eq_diff", "!(p1 == p3)");

    /* [] accessor */
    test_bool(L, "bracket_sym", "p1[:x] == 10 && p1[:y] == 20");
    test_bool(L, "bracket_idx", "p1[0] == 10 && p1[1] == 20");

    /* []= mutator */
    run(L, "p1[:x] = 99");
    test_bool(L, "bracket_set", "p1.x == 99");

    /* writer */
    run(L, "p1.x = 42");
    test_bool(L, "writer", "p1.x == 42");

    /* each */
    run(L, "vals = []");
    run(L, "point.new(1, 2).each { |v| array_push(vals, v) }");
    test_bool(L, "each", "vals[0] == 1 && vals[1] == 2");

    /* to_h */
    run(L, "h = point.new(3, 4).to_h");
    test_bool(L, "to_h", "h[:x] == 3 && h[:y] == 4");

    /* 3-field struct */
    run(L, "color = Struct.new(:r, :g, :b)");
    run(L, "c = color.new(255, 128, 0)");
    test_bool(L, "3field_read", "c.r == 255 && c.g == 128 && c.b == 0");
    test_bool(L, "3field_to_a", "r = c.to_a; r.length == 3 && r[2] == 0");

    printf("\n=== Enumerable Tests ===\n");

    /* Define a class that includes Enumerable */
    run(L,
        "class Numbers\n"
        "  include Enumerable\n"
        "  def initialize(arr)\n"
        "    @data = arr\n"
        "  end\n"
        "  def each(&blk)\n"
        "    i = 0\n"
        "    while i < @data.length\n"
        "      blk.call(@data[i])\n"
        "      i = i + 1\n"
        "    end\n"
        "  end\n"
        "end\n"
    );

    run(L, "nums = Numbers.new([10, 20, 30, 40, 50])");

    test_bool(L, "to_a", "r = nums.to_a; r.length == 5 && r[0] == 10 && r[4] == 50");
    test_bool(L, "entries", "r = nums.entries; r.length == 5 && r[0] == 10");
    test_bool(L, "map", "r = nums.map { |x| x * 2 }; r[0] == 20 && r[4] == 100");
    test_bool(L, "collect", "r = nums.collect { |x| x + 1 }; r[0] == 11 && r[4] == 51");
    test_bool(L, "select", "r = nums.select { |x| x > 25 }; r.length == 3 && r[0] == 30");
    test_bool(L, "reject", "r = nums.reject { |x| x > 25 }; r.length == 2 && r[0] == 10");
    test_bool(L, "find", "r = nums.find { |x| x > 25 }; r == 30");
    test_bool(L, "count", "nums.count == 5");
    test_bool(L, "count_block", "nums.count { |x| x > 25 } == 3");
    test_bool(L, "include?", "nums.include?(30) && !nums.include?(99)");
    test_bool(L, "min", "nums.min == 10");
    test_bool(L, "max", "nums.max == 50");
    test_bool(L, "sum", "nums.sum == 150");
    test_bool(L, "reduce", "nums.reduce(0) { |acc, x| acc + x } == 150");
    test_bool(L, "any?", "nums.any? { |x| x == 30 }");
    test_bool(L, "all?", "nums.all? { |x| x > 0 }");
    test_bool(L, "none?", "nums.none? { |x| x < 0 }");
    test_bool(L, "min_by", "nums.min_by { |x| 0 - x } == 50");
    test_bool(L, "max_by", "nums.max_by { |x| 0 - x } == 10");

    run(L, "unordered = Numbers.new([30, 10, 50, 20, 40])");
    test_bool(L, "sort", "r = unordered.sort; r[0] == 10 && r[1] == 20 && r[4] == 50");
    test_bool(L, "sort_by", "r = nums.sort_by { |x| 0 - x }; r[0] == 50 && r[4] == 10");

    test_bool(L, "flat_map", "r = nums.flat_map { |x| [x, x + 1] }; r.length == 10 && r[0] == 10 && r[1] == 11");
    run(L, "indices = []; nums.each_with_index { |x, idx| array_push(indices, idx) }");
    test_bool(L, "each_with_index", "indices.length == 5 && indices[0] == 0 && indices[4] == 4");
    test_bool(L, "first", "r = nums.first(3); r.length == 3 && r[0] == 10 && r[2] == 30");
    test_bool(L, "take", "r = nums.take(2); r.length == 2 && r[0] == 10 && r[1] == 20");
    test_bool(L, "drop", "r = nums.drop(3); r.length == 2 && r[0] == 40 && r[1] == 50");
    test_bool(L, "group_by", "r = nums.group_by { |x| x > 25 }; length(r[true]) == 3 && length(r[false]) == 2");

    run(L, "dups = Numbers.new([1, 2, 2, 3, 3, 3])");
    test_bool(L, "tally", "r = dups.tally; r[1] == 1 && r[2] == 2 && r[3] == 3");

    run(L, "small = Numbers.new([1, 2, 3])");
    test_bool(L, "zip", "r = small.zip([10, 20, 30]); r.length == 3 && r[0][0] == 1 && r[0][1] == 10");
    test_bool(L, "each_with_object", "r = nums.each_with_object([]) { |x, arr| array_push(arr, x * 10) }; r.length == 5 && r[0] == 100");

    printf("\n=== Comparable Tests ===\n");
    fflush(stdout);

    printf("  [defining Temp class]\n"); fflush(stdout);
    run(L,
        "class Temp\n"
        "  include Comparable\n"
        "  def initialize(deg)\n"
        "    @deg = deg\n"
        "  end\n"
        "  def <=>(other)\n"
        "    @deg <=> other.deg\n"
        "  end\n"
        "  def deg\n"
        "    @deg\n"
        "  end\n"
        "end\n"
    );

    printf("  [creating t1]\n"); fflush(stdout);
    run(L, "t1 = Temp.new(10)");
    printf("  [creating t2]\n"); fflush(stdout);
    run(L, "t2 = Temp.new(20)");
    printf("  [creating t3]\n"); fflush(stdout);
    run(L, "t3 = Temp.new(10)");
    printf("  [test: t1 < t2]\n"); fflush(stdout);
    test_bool(L, "comparable_lt", "t1 < t2");
    test_bool(L, "comparable_gt", "t2 > t1");
    test_bool(L, "comparable_eq", "t1 == t3");
    test_bool(L, "comparable_lte", "t1 <= t3 && t1 <= t2");
    test_bool(L, "comparable_gte", "t2 >= t1 && t1 >= t3");
    test_bool(L, "between?", "t1.between?(Temp.new(5), Temp.new(15))");
    test_bool(L, "clamp", "Temp.new(0).clamp(Temp.new(5), Temp.new(15)).deg == 5");

    printf("\n%d passed, %d failed\n", pass_count, fail_count);
    luby_free(L);
    return fail_count ? 1 : 0;
}
