#define LUBY_IMPLEMENTATION
#include "luby.h"

int main(void) {
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    
    luby_value out;
    
    printf("Test: prime sieve\n");
    luby_eval(L, "n = 30\n sieve = []\n i = 0\n while i <= n\n  sieve[i] = 1\n  i = i + 1\n end\n sieve[0] = 0\n sieve[1] = 0\n p = 2\n while p * p <= n\n  if sieve[p] == 1\n   j = p * p\n   while j <= n\n    sieve[j] = 0\n    j = j + p\n   end\n  end\n  p = p + 1\n end\n count = 0\n i = 2\n while i <= n\n  if sieve[i] == 1\n   count = count + 1\n  end\n  i = i + 1\n end\n count", 0, "<test>", &out);
    
    printf("All done!\n");
    
    luby_free(L);
    return 0;
}
