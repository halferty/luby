#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>

static int test(luby_state *L, const char *name, const char *code) {
    luby_value out;
    int rc = luby_eval(L, code, 0, "<test>", &out);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL %s: %s\n", name, buf);
        return 0;
    }
    printf("PASS %s\n", name);
    return 1;
}

int main(void) {
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    
    // is_a? tests
    test(L, "is_a? basic", "class Foo; end; obj = Foo.new; is_a?(obj, Foo)");
    test(L, "is_a? inheritance", "class Bar; end; class Baz < Bar; end; obj = Baz.new; is_a?(obj, Bar)");
    test(L, "is_a? negative", "class Foo; end; class Bar; end; obj = Foo.new; !is_a?(obj, Bar)");
    
    // kind_of? (should be same as is_a?)
    test(L, "kind_of? basic", "class Foo; end; obj = Foo.new; kind_of?(obj, Foo)");
    
    // instance_of? (exact class only)
    test(L, "instance_of? exact", "class Foo; end; obj = Foo.new; instance_of?(obj, Foo)");
    test(L, "instance_of? not parent", "class Bar; end; class Baz < Bar; end; obj = Baz.new; !instance_of?(obj, Bar)");
    test(L, "instance_of? is subclass", "class Bar; end; class Baz < Bar; end; obj = Baz.new; instance_of?(obj, Baz)");
    
    // defined? tests
    test(L, "defined? global exists", "x = 5; defined?(:x)");
    test(L, "defined? method exists", "defined?(:puts)");
    test(L, "defined? nonexistent", "!defined?(:nonexistent_var_xyz)");
    
    luby_free(L);
    return 0;
}
