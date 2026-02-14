/**
 * Test attr_reader, attr_writer, attr_accessor
 */
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

    printf("=== attr_reader Tests ===\n");

    run(L,
        "class ReaderOnly\n"
        "  attr_reader :name, :age\n"
        "  def initialize(n, a)\n"
        "    @name = n\n"
        "    @age = a\n"
        "  end\n"
        "end\n");

    run(L, "ro = ReaderOnly.new(\"Alice\", 30)");

    test_string(L, "attr_reader name", "ro.name", "Alice");
    test_int(L, "attr_reader age", "ro.age", 30);

    printf("\n=== attr_writer Tests ===\n");

    run(L,
        "class WriterOnly\n"
        "  attr_writer :score\n"
        "  def initialize\n"
        "    @score = 0\n"
        "  end\n"
        "  def get_score\n"
        "    @score\n"
        "  end\n"
        "end\n");

    run(L, "wo = WriterOnly.new");
    test_int(L, "attr_writer initial", "wo.get_score", 0);

    run(L, "wo.score = 100");
    test_int(L, "attr_writer set", "wo.get_score", 100);

    run(L, "wo.score = 42");
    test_int(L, "attr_writer overwrite", "wo.get_score", 42);

    printf("\n=== attr_accessor Tests ===\n");

    run(L,
        "class Coord\n"
        "  attr_accessor :x, :y\n"
        "  def initialize(x, y)\n"
        "    @x = x\n"
        "    @y = y\n"
        "  end\n"
        "end\n");

    run(L, "c = Coord.new(10, 20)");

    test_int(L, "accessor read x", "c.x", 10);
    test_int(L, "accessor read y", "c.y", 20);

    run(L, "c.x = 99");
    test_int(L, "accessor write x", "c.x", 99);

    run(L, "c.y = 55");
    test_int(L, "accessor write y", "c.y", 55);

    test_bool(L, "accessor both", "c.x == 99 && c.y == 55");

    printf("\n=== attr_accessor multiple fields ===\n");

    run(L,
        "class Person\n"
        "  attr_accessor :first, :last, :email\n"
        "  def initialize(f, l, e)\n"
        "    @first = f\n"
        "    @last = l\n"
        "    @email = e\n"
        "  end\n"
        "end\n");

    run(L, "p = Person.new(\"John\", \"Doe\", \"john@example.com\")");
    test_string(L, "multi accessor first", "p.first", "John");
    test_string(L, "multi accessor last", "p.last", "Doe");
    test_string(L, "multi accessor email", "p.email", "john@example.com");

    run(L, "p.first = \"Jane\"");
    test_string(L, "multi accessor write", "p.first", "Jane");

    printf("\n=== attr_accessor with inheritance ===\n");

    run(L,
        "class Base\n"
        "  attr_accessor :val\n"
        "  def initialize(v)\n"
        "    @val = v\n"
        "  end\n"
        "end\n"
        "class Child < Base\n"
        "  attr_accessor :extra\n"
        "  def initialize(v, e)\n"
        "    super(v)\n"
        "    @extra = e\n"
        "  end\n"
        "end\n");

    run(L, "ch = Child.new(1, 2)");
    test_int(L, "inherited accessor val", "ch.val", 1);
    test_int(L, "child accessor extra", "ch.extra", 2);

    run(L, "ch.val = 10");
    run(L, "ch.extra = 20");
    test_bool(L, "inherited write both", "ch.val == 10 && ch.extra == 20");

    printf("\n%d passed, %d failed\n", pass_count, fail_count);
    luby_free(L);
    return fail_count ? 1 : 0;
}
