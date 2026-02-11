#define LUBY_IMPLEMENTATION
#include "../luby.h"

int main(void) {
    luby_config cfg = {0};
    luby_state *L = luby_new(&cfg);
    if (!L) return 1;

    luby_open_base(L);

    const char *code = "print(1 + 2)\nputs(\"hi\")";
    luby_value out;
    if (luby_eval(L, code, 0, "<main>", &out) != 0) {
        luby_error err = luby_last_error(L);
        (void)err;
    }

    luby_free(L);
    return 0;
}
