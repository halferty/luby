/**
 * Test visibility modifiers and alias
 */
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
    
    // Basic visibility tests
    test(L, "public method (default)", 
         "class Foo; def bar; 42; end; end; Foo.new.bar == 42");
    
    test(L, "private method declaration",
         "class Foo\n"
         "  private\n"
         "  def secret; 99; end\n"
         "  public\n"
         "  def use_secret; secret; end\n"
         "end\n"
         "Foo.new.use_secret == 99");
    
    test(L, "private with arguments",
         "class Foo\n"
         "  def visible; 1; end\n"
         "  def hidden; 2; end\n"
         "  private :hidden\n"
         "  def test; hidden; end\n"
         "end\n"
         "Foo.new.test == 2");
    
    test(L, "public with arguments",
         "class Foo\n"
         "  private\n"
         "  def was_private; 3; end\n"
         "  public :was_private\n"
         "end\n"
         "Foo.new.was_private == 3");
    
    test(L, "protected method",
         "class Foo\n"
         "  protected\n"
         "  def prot; 5; end\n"
         "  public\n"
         "  def use_prot; prot; end\n"
         "end\n"
         "Foo.new.use_prot == 5");
    
    // Alias tests
    test(L, "alias basic",
         "class Foo\n"
         "  def original; 10; end\n"
         "  alias aliased original\n"
         "end\n"
         "Foo.new.aliased == 10");
    
    test(L, "alias with symbols",
         "class Bar\n"
         "  def first; 20; end\n"
         "  alias :second :first\n"
         "end\n"
         "Bar.new.second == 20");
    
    test(L, "multiple aliases",
         "class Baz\n"
         "  def add(a, b); a + b; end\n"
         "  alias plus add\n"
         "  alias sum add\n"
         "end\n"
         "obj = Baz.new\n"
         "obj.add(1, 2) == 3 && obj.plus(1, 2) == 3 && obj.sum(1, 2) == 3");
    
    // Combined tests
    test(L, "alias preserves visibility",
         "class Test\n"
         "  private\n"
         "  def secret; 42; end\n"
         "  alias hidden secret\n"
         "  public\n"
         "  def reveal; hidden; end\n"
         "end\n"
         "Test.new.reveal == 42");
    
    test(L, "visibility reset per class",
         "class A\n"
         "  private\n"
         "  def foo; 1; end\n"
         "end\n"
         "class B\n"
         "  def bar; 2; end\n"
         "end\n"
         "B.new.bar == 2");
    
    luby_free(L);
    return 0;
}
