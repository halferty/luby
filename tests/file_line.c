/**
 * Test __FILE__ and __LINE__ special variables
 */
#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

static int test(luby_state *L, const char *name, const char *code, const char *filename) {
    luby_value out;
    int rc = luby_eval(L, code, 0, filename, &out);
    tests_run++;
    printf("  %s ... ", name);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAILED: %s\n", buf);
        return 0;
    }
    printf("ok\n");
    tests_passed++;
    return 1;
}

int main(void) {
    printf("Testing __FILE__ and __LINE__...\n");

    luby_state *L = luby_new(NULL);
    if (!L) {
        fprintf(stderr, "Failed to create luby_state\n");
        return 1;
    }
    luby_open_base(L);

    // Simple tests
    test(L, "__FILE__ returns filename", "__FILE__ == \"test.rb\"", "test.rb");
    test(L, "__LINE__ returns 1", "__LINE__ == 1", "test.rb");
    test(L, "__LINE__ on line 5", "\n\n\n\n__LINE__ == 5", "test.rb");
    test(L, "__FILE__ in expression", "x = __FILE__; x == \"myfile.rb\"", "myfile.rb");
    test(L, "__LINE__ in expression", "y = __LINE__; y == 1", "test.rb");
    
    test(L, "__FILE__ and __LINE__ in method",
         "def info; [__FILE__, __LINE__]; end; r = info; r[0] == \"source.rb\" && r[1] == 2",
         "source.rb");
    
    test(L, "__LINE__ increments per line",
         "a = __LINE__\nb = __LINE__\nc = __LINE__\na == 1 && b == 2 && c == 3",
         "test.rb");
    
    test(L, "__FILE__ in string interpolation",
         "\"File: #{__FILE__}\" == \"File: test.rb\"",
         "test.rb");
    
    test(L, "__LINE__ in string interpolation",
         "\"Line: #{__LINE__}\" == \"Line: 1\"",
         "test.rb");

    luby_free(L);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
