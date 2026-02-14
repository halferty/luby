#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

static int assert_string(const char *label, luby_value v, const char *expected) {
    if (v.type != LUBY_T_STRING) {
        printf("FAIL: %s (got type=%d, expected string)\n", label, v.type);
        tests_failed++;
        return 0;
    }
    const char *actual = v.as.ptr ? (const char *)v.as.ptr : "";
    if (strcmp(actual, expected) != 0) {
        printf("FAIL: %s\n  got: \"%s\"\n  expected: \"%s\"\n", label, actual, expected);
        tests_failed++;
        return 0;
    }
    tests_passed++;
    return 1;
}

static int assert_int(const char *label, luby_value v, int64_t expected) {
    if (v.type != LUBY_T_INT || v.as.i != expected) {
        printf("FAIL: %s (got type=%d, val=%lld, expected %lld)\n", label, v.type, (long long)v.as.i, (long long)expected);
        tests_failed++;
        return 0;
    }
    tests_passed++;
    return 1;
}

static int eval_check(luby_state *L, const char *label, const char *code, luby_value *out) {
    int rc = luby_eval(L, code, 0, "<test>", out);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL: %s (%s)\n", label, buf);
        tests_failed++;
        return 0;
    }
    return 1;
}

static void test_basic_heredoc(void) {
    printf("Testing: basic heredoc\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "x = <<END\n"
        "Hello World\n"
        "END\n"
        "x\n";
    if (eval_check(L, "basic heredoc", code, &result)) {
        assert_string("basic heredoc", result, "Hello World\n");
    }
    luby_free(L);
}

static void test_multiline_heredoc(void) {
    printf("Testing: multiline heredoc\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "x = <<TEXT\n"
        "Line 1\n"
        "Line 2\n"
        "Line 3\n"
        "TEXT\n"
        "x\n";
    if (eval_check(L, "multiline heredoc", code, &result)) {
        assert_string("multiline heredoc", result, "Line 1\nLine 2\nLine 3\n");
    }
    luby_free(L);
}

static void test_heredoc_with_quotes(void) {
    printf("Testing: heredoc with quoted delimiter\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "x = <<\"SQL\"\n"
        "SELECT * FROM users\n"
        "SQL\n"
        "x\n";
    if (eval_check(L, "heredoc quoted delimiter", code, &result)) {
        assert_string("heredoc quoted delimiter", result, "SELECT * FROM users\n");
    }
    luby_free(L);
}

static void test_heredoc_indented_delimiter(void) {
    printf("Testing: heredoc with indented delimiter (<<-)\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "x = <<-END\n"
        "  Some text\n"
        "  END\n"
        "x\n";
    if (eval_check(L, "heredoc indented delimiter", code, &result)) {
        assert_string("heredoc indented delimiter", result, "  Some text\n");
    }
    luby_free(L);
}

static void test_heredoc_string_methods(void) {
    printf("Testing: heredoc with string methods\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "x = <<END\n"
        "hello\n"
        "END\n"
        "x.length\n";
    if (eval_check(L, "heredoc string methods", code, &result)) {
        assert_int("heredoc length", result, 6);  // "hello\n" = 6 chars
    }
    luby_free(L);
}

static void test_heredoc_in_method(void) {
    printf("Testing: heredoc inside method\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "def greet\n"
        "  <<MSG\n"
        "Hello!\n"
        "MSG\n"
        "end\n"
        "greet()\n";  // Need explicit parens to call method
    if (eval_check(L, "heredoc in method", code, &result)) {
        assert_string("heredoc in method", result, "Hello!\n");
    }
    luby_free(L);
}

static void test_heredoc_empty(void) {
    printf("Testing: empty heredoc\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "x = <<END\n"
        "END\n"
        "x\n";
    if (eval_check(L, "empty heredoc", code, &result)) {
        assert_string("empty heredoc", result, "");
    }
    luby_free(L);
}

static void test_heredoc_uppercase_delimiter(void) {
    printf("Testing: heredoc with various delimiters\n");
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    const char *code =
        "x = <<HTML\n"
        "<h1>Title</h1>\n"
        "HTML\n"
        "x\n";
    if (eval_check(L, "heredoc HTML delimiter", code, &result)) {
        assert_string("heredoc HTML delimiter", result, "<h1>Title</h1>\n");
    }
    luby_free(L);
}

int main(void) {
    printf("=== Heredoc String Tests ===\n\n");
    
    test_basic_heredoc();
    test_multiline_heredoc();
    test_heredoc_with_quotes();
    test_heredoc_indented_delimiter();
    test_heredoc_string_methods();
    test_heredoc_in_method();
    test_heredoc_empty();
    test_heredoc_uppercase_delimiter();
    
    printf("\n=== Summary: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
