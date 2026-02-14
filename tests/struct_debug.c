#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>
int main(void) {
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value out;
    luby_eval(L, "point = Struct.new(:x, :y)", 0, "<t>", &out);
    printf("result type=%d\n", out.type);
    luby_free(L);
    return 0;
}
