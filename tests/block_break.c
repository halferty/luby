#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static int tests_passed = 0;
static int tests_failed = 0;

static int eval_check(luby_state *L, const char *label, const char *code, luby_value *out) {
    int rc = luby_eval(L, code, 0, "<test>", out);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL (%s): %s\n", label, buf);
        luby_clear_error(L);
        tests_failed++;
        return 0;
    }
    return 1;
}

#define TEST(label) printf("  %-55s ", label);
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)
#define ASSERT_INT(v, expected) do { \
    assert((v).type == LUBY_T_INT); \
    assert((v).as.i == (expected)); \
} while(0)
#define ASSERT_FLOAT(v, expected) do { \
    assert((v).type == LUBY_T_FLOAT); \
    assert((v).as.f == (expected)); \
} while(0)
#define ASSERT_NIL(v) assert((v).type == LUBY_T_NIL)
#define ASSERT_TRUE(v) assert(luby_is_truthy(v))
#define ASSERT_FALSE(v) assert(!luby_is_truthy(v))

int main() {
    printf("=== Block Break/Next Tests ===\n\n");

    // --- Test 1: break inside each ---
    TEST("break inside each stops iteration") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);
        luby_value out;
        // break from each should return break value
        if (eval_check(L, "break each",
            "result = 0\n"
            "[1,2,3,4,5].each { |x| break if x == 3; result = x }\n"
            "result", &out)) {
            ASSERT_INT(out, 2);
            PASS();
        }
        luby_free(L);
    }

    // --- Test 2: break with value inside each ---
    TEST("break with value inside each returns value") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);
        luby_value out;
        if (eval_check(L, "break value each",
            "r = [1,2,3,4,5].each { |x| break 42 if x == 3 }\n"
            "r", &out)) {
            ASSERT_INT(out, 42);
            PASS();
        }
        luby_free(L);
    }

    // --- Test 3: next inside each ---
    TEST("next inside each skips to next iteration") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);
        luby_value out;
        if (eval_check(L, "next each",
            "count = 0\n"
            "[1,2,3,4,5].each { |x| next if x == 3; count = count + 1 }\n"
            "count", &out)) {
            ASSERT_INT(out, 4);  // skipped x==3, counted 1,2,4,5
            PASS();
        }
        luby_free(L);
    }

    // --- Test 4: next with value inside map ---
    TEST("next with value inside map provides mapped value") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);
        luby_value out;
        if (eval_check(L, "next value map",
            "r = [1,2,3,4].map { |x| next 0 if x == 3; x * 10 }\n"
            "r[2]", &out)) {
            ASSERT_INT(out, 0);  // 3rd element replaced with 0
            PASS();
        }
        luby_free(L);
    }

    // --- Test 5: break inside map ---
    TEST("break inside map stops early") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);
        luby_value out;
        if (eval_check(L, "break map",
            "r = [1,2,3,4,5].map { |x| break 99 if x == 3; x }\n"
            "r", &out)) {
            ASSERT_INT(out, 99);
            PASS();
        }
        luby_free(L);
    }

    // --- Test 6: break inside select ---
    TEST("break inside select stops early") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);
        luby_value out;
        if (eval_check(L, "break select",
            "r = [1,2,3,4,5].select { |x| break -1 if x == 4; x.odd? }\n"
            "r", &out)) {
            ASSERT_INT(out, -1);
            PASS();
        }
        luby_free(L);
    }

    // --- Test 7: next inside select ---
    TEST("next inside select skips element") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);
        luby_value out;
        // next without value in select returns nil (falsy), so element is skipped
        if (eval_check(L, "next select",
            "r = [1,2,3,4,5].select { |x| next if x == 3; true }\n"
            "r.length", &out)) {
            ASSERT_INT(out, 4);  // [1,2,4,5]
            PASS();
        }
        luby_free(L);
    }

    // --- Test 8: break inside each_with_index ---
    TEST("break inside each_with_index") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);
        luby_value out;
        if (eval_check(L, "break each_with_index",
            "result = 0\n"
            "[10,20,30,40].each_with_index { |v, i| break if i == 2; result = v }\n"
            "result", &out)) {
            ASSERT_INT(out, 20);
            PASS();
        }
        luby_free(L);
    }

    // --- Test 9: break inside reduce/inject ---
    TEST("break inside reduce stops early") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);
        luby_value out;
        if (eval_check(L, "break reduce",
            "r = [1,2,3,4,5].reduce(0) { |acc, x| break acc if x == 4; acc + x }\n"
            "r", &out)) {
            ASSERT_INT(out, 6);  // 1+2+3 = 6, break before adding 4
            PASS();
        }
        luby_free(L);
    }

    // --- Test 10: break without value returns nil ---
    TEST("break without value returns nil") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);
        luby_value out;
        if (eval_check(L, "break nil",
            "r = [1,2,3].each { |x| break if x == 2 }\n"
            "r", &out)) {
            ASSERT_NIL(out);
            PASS();
        }
        luby_free(L);
    }

    // --- Test 11: next inside times ---
    TEST("next inside times") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);
        luby_value out;
        if (eval_check(L, "next times",
            "count = 0\n"
            "5.times { |i| next if i == 2; count = count + 1 }\n"
            "count", &out)) {
            ASSERT_INT(out, 4);
            PASS();
        }
        luby_free(L);
    }

    // --- Test 12: break inside times ---
    TEST("break inside times") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);
        luby_value out;
        if (eval_check(L, "break times",
            "count = 0\n"
            "10.times { |i| break if i == 3; count = count + 1 }\n"
            "count", &out)) {
            ASSERT_INT(out, 3);
            PASS();
        }
        luby_free(L);
    }

    // --- Test 13: break inside find ---
    TEST("break inside find") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);
        luby_value out;
        if (eval_check(L, "break find",
            "r = [1,2,3,4,5].find { |x| break 77 if x == 3; false }\n"
            "r", &out)) {
            ASSERT_INT(out, 77);
            PASS();
        }
        luby_free(L);
    }

    // --- Test 14: break inside any? ---
    TEST("break inside any?") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);
        luby_value out;
        if (eval_check(L, "break any",
            "r = [1,2,3,4,5].any? { |x| break \"found\" if x == 3; false }\n"
            "r", &out)) {
            assert(out.type == LUBY_T_STRING);
            assert(strcmp((const char *)out.as.ptr, "found") == 0);
            PASS();
        }
        luby_free(L);
    }

    // --- Test 15: conditional break with value ---
    TEST("conditional break with modifier") {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);
        luby_value out;
        if (eval_check(L, "break cond",
            "result = [1,2,3,4,5].each { |x|\n"
            "  break x * 100 if x > 3\n"
            "}\n"
            "result", &out)) {
            ASSERT_INT(out, 400);
            PASS();
        }
        luby_free(L);
    }

    printf("\n=== Results ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
