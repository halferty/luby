#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>

int main(void) {
    luby_state *L = luby_new(NULL);
    if (!L) return 1;
    luby_open_base(L);

    const char *code = "puts(array_select([1,2,3,4]) { |x| x > 2 })";
    luby_value out;
    int rc = luby_eval(L, code, 0, "<select>", &out);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("Error: %s\n", buf);
    }

    luby_free(L);
    return rc != 0;
}
