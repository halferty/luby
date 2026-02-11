#define LUBY_IMPLEMENTATION
#include "../luby.h"

int main(void) {
    luby_state *L = luby_new(NULL);
    if (!L) return 1;
    luby_open_base(L);

    const char *code = "array_map([1,2,3]) { |x| x * 2 }";
    luby_value out;
    if (luby_eval(L, code, 0, "<main>", &out) != 0) {
        luby_error err = luby_last_error(L);
        (void)err;
    }

    // Print result
    luby_call(L, luby_nil(), "puts", 1, &out, NULL);

    luby_free(L);
    return 0;
}
