#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>

static int test_int(luby_state *L, const char *name, const char *code, int expected) {
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

static int test_nil(luby_state *L, const char *name, const char *code) {
    luby_value out;
    int rc = luby_eval(L, code, 0, "<test>", &out);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL %s: %s\n", name, buf);
        return 0;
    }
    if (out.type != LUBY_T_NIL) {
        printf("FAIL %s: expected nil, got ", name);
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

    // === Basic class variable set and get ===
    ok &= test_int(L, "cvar basic set/get",
        "class Counter; @@count = 0; def self.count; @@count; end; def self.inc; @@count = @@count + 1; end; end; Counter.inc; Counter.count", 1);

    // === Class variable shared across instances ===
    ok &= test_int(L, "cvar shared across instances",
        "class Tracker; @@total = 0; def add(n); @@total = @@total + n; end; def total; @@total; end; end; "
        "t1 = Tracker.new; t2 = Tracker.new; t1.add(5); t2.add(3); t1.total", 8);

    // === Class variable initialized in class body ===
    ok &= test_int(L, "cvar init in class body",
        "class Config; @@value = 42; def self.value; @@value; end; end; Config.value", 42);

    // === Class variable with ||= ===
    ok &= test_int(L, "cvar with ||=",
        "class Cache; def self.get; @@data ||= 100; end; end; Cache.get", 100);

    // === Class variable persists ===
    ok &= test_int(L, "cvar persists",
        "class Persist; @@x = 1; def self.inc; @@x = @@x + 1; end; def self.get; @@x; end; end; "
        "Persist.inc; Persist.inc; Persist.get", 3);

    // === Class variable with +=  ===
    ok &= test_int(L, "cvar with +=",
        "class Adder; @@sum = 0; def self.add(n); @@sum += n; end; def self.sum; @@sum; end; end; "
        "Adder.add(10); Adder.add(20); Adder.sum", 30);

    // === Uninitialized class variable returns nil ===
    ok &= test_nil(L, "cvar uninitialized returns nil",
        "class Empty; def self.x; @@x; end; end; Empty.x");

    // === Class variable inheritance - subclass sees parent cvar via instance methods ===
    ok &= test_int(L, "cvar inheritance read",
        "class Parent; @@shared = 99; def shared; @@shared; end; end; "
        "class Child < Parent; end; Child.new.shared", 99);

    // === Class variable inheritance - subclass can modify parent cvar via instance methods ===
    ok &= test_int(L, "cvar inheritance modify",
        "class Base; @@val = 10; def val; @@val; end; def set(v); @@val = v; end; end; "
        "class Derived < Base; end; d = Derived.new; d.set(50); Base.new.val", 50);

    // === Multiple class variables ===
    ok &= test_int(L, "multiple cvars",
        "class Multi; @@a = 1; @@b = 2; @@c = 3; def self.sum; @@a + @@b + @@c; end; end; Multi.sum", 6);

    // === Class variable in instance method ===
    ok &= test_int(L, "cvar in instance method",
        "class Instance; @@count = 0; def initialize; @@count = @@count + 1; end; def self.count; @@count; end; end; "
        "Instance.new; Instance.new; Instance.new; Instance.count", 3);

    luby_free(L);
    printf("\ncvar tests: %s\n", ok ? "ALL PASSED" : "SOME FAILED");
    return ok ? 0 : 1;
}
