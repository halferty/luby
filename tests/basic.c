#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>
#include <string.h>

static int assert_int(const char *label, luby_value v, int64_t expected) {
    if (v.type != LUBY_T_INT || v.as.i != expected) {
        printf("FAIL: %s (got type=%d, val=%lld)\n", label, v.type, (long long)v.as.i);
        return 0;
    }
    return 1;
}

static int assert_string(const char *label, luby_value v, const char *expected) {
    if (v.type != LUBY_T_STRING) {
        printf("FAIL: %s (got type=%d, expected string)\n", label, v.type);
        return 0;
    }
    const char *actual = v.as.ptr ? (const char *)v.as.ptr : "";
    if (strcmp(actual, expected) != 0) {
        printf("FAIL: %s (got \"%s\", expected \"%s\")\n", label, actual, expected);
        return 0;
    }
    return 1;
}

static int eval_check(luby_state *L, const char *label, const char *code, luby_value *out) {
    int rc = luby_eval(L, code, 0, "<test>", out);
    if (rc != 0) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("FAIL: %s (%s)\n", label, buf);
        return 0;
    }
    return 1;
}

// Native method for luby_define_method tests: double(n) returns n * 2
static int native_double(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc >= 2 && argv[1].type == LUBY_T_INT) {
        out->type = LUBY_T_INT;
        out->as.i = argv[1].as.i * 2;
    } else {
        *out = luby_nil();
    }
    return 0;
}

// Native method for luby_define_method tests: add(a, b) returns a + b
static int native_add(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    (void)L;
    if (argc >= 3 && argv[1].type == LUBY_T_INT && argv[2].type == LUBY_T_INT) {
        out->type = LUBY_T_INT;
        out->as.i = argv[1].as.i + argv[2].as.i;
    } else {
        *out = luby_nil();
    }
    return 0;
}

static int native_yield_cfunc(luby_state *L, int argc, const luby_value *argv, luby_value *out) {
    luby_value v = (argc > 0) ? argv[0] : luby_int(1);
    if (out) *out = luby_nil();
    return luby_native_yield(L, v);
}

int main(void) {
    int ok = 1;
    luby_state *L = luby_new(NULL);
    if (!L) return 1;
    luby_open_base(L);
    luby_register_function(L, "native_yield", native_yield_cfunc);

    luby_value out;

    // arithmetic
    if (eval_check(L, "arith", "1 + 2 * 3", &out)) {
        ok &= assert_int("arith", out, 7);
    } else {
        ok = 0;
    }

    // arrays + index
    if (eval_check(L, "array index", "a = [1,2,3]; a[1]", &out)) {
        ok &= assert_int("array index", out, 2);
    } else {
        ok = 0;
    }

    // array index assign
    if (eval_check(L, "array index assign", "a = [1,2,3]; a[1] = 9; a[1]", &out)) {
        ok &= assert_int("array index assign", out, 9);
    } else {
        ok = 0;
    }

    // hash
    if (eval_check(L, "hash index", "h = {\"x\" => 9}; h[\"x\"]", &out)) {
        ok &= assert_int("hash index", out, 9);
    } else {
        ok = 0;
    }

    // hash set via index
    if (eval_check(L, "hash index assign", "h = {}; h[\"k\"] = 7; h[\"k\"]", &out)) {
        ok &= assert_int("hash index assign", out, 7);
    } else {
        ok = 0;
    }

    // hash helpers
    if (eval_check(L, "hash_get", "h = {\"a\" => 1}; hash_get(h, \"a\")", &out)) {
        ok &= assert_int("hash_get", out, 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "hash_set", "h = {}; hash_set(h, \"a\", 2); h[\"a\"]", &out)) {
        ok &= assert_int("hash_set", out, 2);
    } else {
        ok = 0;
    }

    // hash enumerables
    if (eval_check(L, "hash_each", "h = {\"a\" => 1, \"b\" => 2}; sum = 0; hash_each(h) { |k, v| sum = sum + v }; sum", &out)) {
        ok &= assert_int("hash_each", out, 3);
    } else {
        ok = 0;
    }
    if (eval_check(L, "hash_map", "h = {\"a\" => 1, \"b\" => 2}; len(hash_map(h) { |k, v| v * 2 })", &out)) {
        ok &= assert_int("hash_map", out, 2);
    } else {
        ok = 0;
    }
    if (eval_check(L, "hash_select", "h = {\"a\" => 1, \"b\" => 2, \"c\" => 3}; hs = hash_select(h) { |k, v| v > 1 }; len(hs)", &out)) {
        ok &= assert_int("hash_select", out, 2);
    } else {
        ok = 0;
    }
    if (eval_check(L, "hash_reject", "h = {\"a\" => 1, \"b\" => 2, \"c\" => 3}; hr = hash_reject(h) { |k, v| v > 1 }; len(hr)", &out)) {
        ok &= assert_int("hash_reject", out, 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "hash_any", "h = {\"a\" => 1, \"b\" => 2}; hash_any(h) { |k, v| v == 2 }", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "hash_all", "h = {\"a\" => 1, \"b\" => 2}; hash_all(h) { |k, v| v > 0 }", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "hash_none", "h = {\"a\" => 1, \"b\" => 2}; hash_none(h) { |k, v| v < 0 }", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "hash_find", "h = {\"a\" => 1, \"b\" => 2, \"c\" => 3}; k = hash_find(h) { |key, v| v == 2 }; hash_get(h, k)", &out)) {
        ok &= assert_int("hash_find", out, 2);
    } else {
        ok = 0;
    }
    if (eval_check(L, "hash_reduce", "h = {\"a\" => 1, \"b\" => 2, \"c\" => 3}; hash_reduce(h, 0) { |acc, k, v| acc + v }", &out)) {
        ok &= assert_int("hash_reduce", out, 6);
    } else {
        ok = 0;
    }

    // block map
    if (eval_check(L, "block map", "array_map([1,2,3]) { |x| x * 2 }", &out)) {
        // Expect array length 3
        ok &= (luby_array_len(out) == 3);
    } else {
        ok = 0;
    }

    // method-style map
    if (eval_check(L, "map method", "len([1,2,3].map { |x| x + 1 })", &out)) {
        ok &= assert_int("map method", out, 3);
    } else {
        ok = 0;
    }
    if (eval_check(L, "array select", "len(select([1,2,3]) { |x| x > 1 })", &out)) {
        ok &= assert_int("array select", out, 2);
    } else {
        ok = 0;
    }
    if (eval_check(L, "array reject", "len(reject([1,2,3]) { |x| x > 1 })", &out)) {
        ok &= assert_int("array reject", out, 1);
    } else {
        ok = 0;
    }

    // each
    if (eval_check(L, "each", "sum = 0; each([1,2,3]) { |x| sum = sum + x }; sum", &out)) {
        ok &= assert_int("each", out, 6);
    } else {
        ok = 0;
    }
    if (eval_check(L, "enumerator next", "e = each([1,2,3]); a = e.next(); b = e.next(); a + b", &out)) {
        ok &= assert_int("enumerator next", out, 3);
    } else {
        ok = 0;
    }

    if (eval_check(L, "enumerator rewind", "e = each([1,2]); e.next(); e.rewind(); e.next()", &out)) {
        ok &= assert_int("enumerator rewind", out, 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "enumerator each_with_index", "e = each_with_index([10,20]); a = e.next(); b = e.next(); a[0] + a[1] + b[0] + b[1]", &out)) {
        ok &= assert_int("enumerator each_with_index", out, 31);
    } else {
        ok = 0;
    }
    if (eval_check(L, "hash enumerator", "e = hash_each({\"a\" => 1, \"b\" => 2}); p = e.next(); len(p)", &out)) {
        ok &= assert_int("hash enumerator", out, 2);
    } else {
        ok = 0;
    }
    if (eval_check(L, "coroutine single", "c = coroutine_new { 42 }\n v = coroutine_resume(c)\n v", &out)) {
        ok &= assert_int("coroutine single", out, 42);
    } else {
        ok = 0;
    }
    if (eval_check(L, "coroutine alive", "c = coroutine_new { 1 }\n a = coroutine_alive(c)\n coroutine_resume(c)\n b = coroutine_alive(c)\n a && (!b)", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "coroutine yield", "c = coroutine_new { yield(7) }\n v = coroutine_resume(c)\n v", &out)) {
        ok &= assert_int("coroutine yield", out, 7);
    } else {
        ok = 0;
    }
    if (eval_check(L, "coroutine multi yield", "c = coroutine_new { yield(1); yield(2); yield(3); 9 }\n a = coroutine_resume(c)\n b = coroutine_resume(c)\n d = coroutine_resume(c)\n e = coroutine_resume(c)\n a * 100 + b * 10 + d + e", &out)) {
        ok &= assert_int("coroutine multi yield", out, 132);
    } else {
        ok = 0;
    }
    if (eval_check(L, "coroutine yield in", "c = coroutine_new { x = yield(1); y = yield(x + 1); y }\n a = coroutine_resume(c)\n b = coroutine_resume(c, 10)\n d = coroutine_resume(c, 7)\n a + b + d", &out)) {
        ok &= assert_int("coroutine yield in", out, 19);
    } else {
        ok = 0;
    }
    if (eval_check(L, "coroutine nested", "c = coroutine_new { a = 1; b = 2; yield(b); a + b }\n coroutine_resume(c)\n b = coroutine_resume(c)\n b", &out)) {
        ok &= assert_int("coroutine nested", out, 3);
    } else {
        ok = 0;
    }
    if (eval_check(L, "coroutine alive yields", "c = coroutine_new { yield(1); yield(2); 3 }\n a = coroutine_alive(c)\n coroutine_resume(c)\n b = coroutine_alive(c)\n coroutine_resume(c)\n coroutine_resume(c)\n d = coroutine_alive(c)\n (a && b) && (!d)", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "coroutine native yield", "c = coroutine_new { x = native_yield(5); x + 1 }\n a = coroutine_resume(c)\n b = coroutine_resume(c, 9)\n a * 10 + b", &out)) {
        ok &= assert_int("coroutine native yield", out, 60);
    } else {
        ok = 0;
    }

    // array push/pop
    if (eval_check(L, "array_push", "a = [1]; array_push(a, 2); len(a)", &out)) {
        ok &= assert_int("array_push", out, 2);
    } else {
        ok = 0;
    }
    if (eval_check(L, "array_pop", "a = [1,2]; array_pop(a)", &out)) {
        ok &= assert_int("array_pop", out, 2);
    } else {
        ok = 0;
    }
    if (eval_check(L, "for loop", "sum = 0\n for x in [1,2,3]\n  sum = sum + x\n end\n sum", &out)) {
        ok &= assert_int("for loop", out, 6);
    } else {
        ok = 0;
    }
        if (eval_check(L, "each_with_index", "a = [1,2,3]; sum = 0; each_with_index(a) { |v, i| sum = sum + v * i }; sum", &out)) {
            ok &= assert_int("each_with_index", out, 8);
        } else {
            ok = 0;
        }
        if (eval_check(L, "compact", "a = [1, nil, 2, nil, 3]; b = compact(a); len(a) + len(b)", &out)) {
            ok &= assert_int("compact", out, 8);
        } else {
            ok = 0;
        }
        if (eval_check(L, "compact!", "a = [1, nil, 2, nil]; compact!(a); len(a)", &out)) {
            ok &= assert_int("compact!", out, 2);
        } else {
            ok = 0;
        }
        if (eval_check(L, "hash merge", "h1 = {\"a\" => 1, \"b\" => 2}; h2 = {\"b\" => 3, \"c\" => 4}; h3 = merge(h1, h2); h1[\"b\"] + h3[\"b\"]", &out)) {
            ok &= assert_int("hash merge", out, 5);
        } else {
            ok = 0;
        }

    // if/while
    if (eval_check(L, "while", "i = 0; while i < 3\n i = i + 1\n end\n i", &out)) {
        ok &= assert_int("while", out, 3);
    } else {
        ok = 0;
    }

    if (eval_check(L, "until", "i = 0; until i >= 3\n i = i + 1\n end\n i", &out)) {
        ok &= assert_int("until", out, 3);
    } else {
        ok = 0;
    }

    if (eval_check(L, "if", "if 1 < 2\n 10\n else\n 20\n end", &out)) {
        ok &= assert_int("if", out, 10);
    } else {
        ok = 0;
    }

    if (eval_check(L, "unless", "unless 1 > 2\n 11\n else\n 22\n end", &out)) {
        ok &= assert_int("unless", out, 11);
    } else {
        ok = 0;
    }

    // break/next
    if (eval_check(L, "next", "i = 0; sum = 0; while i < 5\n i = i + 1\n if i == 3\n  next\n end\n sum = sum + i\n end\n sum", &out)) {
        ok &= assert_int("next", out, 12);
    } else {
        ok = 0;
    }
    if (eval_check(L, "break", "i = 0; while i < 5\n i = i + 1\n if i == 3\n  break 7\n end\n end", &out)) {
        ok &= assert_int("break", out, 7);
    } else {
        ok = 0;
    }

    if (eval_check(L, "redo", "i = 0; sum = 0; redoed = 0; while i < 3\n i = i + 1\n if i == 1 && redoed == 0\n  redoed = 1\n  redo\n end\n sum = sum + i\n end\n sum", &out)) {
        ok &= assert_int("redo", out, 5);
    } else {
        ok = 0;
    }

    // case/when
    if (eval_check(L, "case", "x = 2\n case x\n when 1\n  10\n when 2, 3\n  20\n else\n  30\n end", &out)) {
        ok &= assert_int("case", out, 20);
    } else {
        ok = 0;
    }

    // def
    if (eval_check(L, "def", "def add(a, b)\n a + b\n end\n add(2,3)", &out)) {
        ok &= assert_int("def", out, 5);
    } else {
        ok = 0;
    }

    // lambda/block args
    if (eval_check(L, "block args", "sum = 0; each([1,2,3]) { |x| sum = sum + x }; sum", &out)) {
        ok &= assert_int("block args", out, 6);
    } else {
        ok = 0;
    }

    // class/module parsing smoke (no runtime yet)
    if (eval_check(L, "class method", "class Foo\n def bar()\n 1\n end\n end\n Foo.new.bar", &out)) {
        ok &= assert_int("class method", out, 1);
    } else {
        ok = 0;
    }

    // mixins
    if (eval_check(L, "include", "module M\n def foo()\n 9\n end\n end\n class C\n include M\n end\n C.new.foo", &out)) {
        ok &= assert_int("include", out, 9);
    } else {
        ok = 0;
    }
    if (eval_check(L, "extend", "module N\n def bar()\n 8\n end\n end\n class D\n end\n d = D.new\n extend(d, N)\n d.bar", &out)) {
        ok &= assert_int("extend", out, 8);
    } else {
        ok = 0;
    }
    if (eval_check(L, "include in module", "module M1\n def a()\n 1\n end\n end\n module M2\n include M1\n def b()\n 2\n end\n end\n class C2\n include M2\n end\n C2.new.a + C2.new.b", &out)) {
        ok &= assert_int("include in module", out, 3);
    } else {
        ok = 0;
    }
    if (eval_check(L, "extend class", "module CM\n def cls()\n 5\n end\n end\n class CC\n end\n extend(CC, CM)\n CC.cls", &out)) {
        ok &= assert_int("extend class", out, 5);
    } else {
        ok = 0;
    }

    // exceptions
    if (eval_check(L, "rescue", "begin\n raise(\"oops\")\n rescue\n 7\n end", &out)) {
        ok &= assert_int("rescue", out, 7);
    } else {
        ok = 0;
    }
    if (eval_check(L, "ensure", "x = 0\n begin\n x = 1\n ensure\n x = x + 1\n end\n x", &out)) {
        ok &= assert_int("ensure", out, 2);
    } else {
        ok = 0;
    }
    if (eval_check(L, "rescue+ensure", "x = 0\n begin\n raise \"boom\"\n rescue\n x = 1\n ensure\n x = x + 1\n end\n x", &out)) {
        ok &= assert_int("rescue+ensure", out, 2);
    } else {
        ok = 0;
    }

    // inheritance + super
    if (eval_check(L, "super", "class A\n def val()\n 1\n end\n end\n class B < A\n def val()\n super() + 1\n end\n end\n B.new.val", &out)) {
        ok &= assert_int("super", out, 2);
    } else {
        ok = 0;
    }

    // respond_to on class/module
    if (eval_check(L, "respond_to class", "class E\n def foo()\n 1\n end\n end\n respond_to(E, \"foo\")", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }

    // send/public_send
    if (eval_check(L, "send", "class S\n def foo(x)\n x + 1\n end\n end\n s = S.new\n send(s, \"foo\", 2)", &out)) {
        ok &= assert_int("send", out, 3);
    } else {
        ok = 0;
    }
    if (eval_check(L, "public_send", "class S2\n def bar(x)\n x * 2\n end\n end\n s = S2.new\n public_send(s, \"bar\", 3)", &out)) {
        ok &= assert_int("public_send", out, 6);
    } else {
        ok = 0;
    }

    // define_method
    if (eval_check(L, "define_method", "class DM\n define_method(\"baz\") { 5 }\n end\n DM.new.baz", &out)) {
        ok &= assert_int("define_method", out, 5);
    } else {
        ok = 0;
    }

    // respond_to_missing?
    if (eval_check(L, "respond_to_missing", "class RM\n def respond_to_missing?(name)\n true\n end\n end\n respond_to(RM.new, \"nope\")", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }

    // included/inherited hooks
    if (eval_check(L, "included hook", "module MHook\n def included(klass)\n x = 7\n end\n end\n class CH\n include MHook\n end\n x", &out)) {
        ok &= assert_int("included hook", out, 7);
    } else {
        ok = 0;
    }
    if (eval_check(L, "inherited hook", "class P\n def inherited(klass)\n y = 9\n end\n end\n class Q < P\n end\n y", &out)) {
        ok &= assert_int("inherited hook", out, 9);
    } else {
        ok = 0;
    }

    // class_eval / instance_eval
    if (eval_check(L, "class_eval string", "class CE\n end\n class_eval(CE, \"def foo()\n 7\n end\")\n CE.new.foo", &out)) {
        ok &= assert_int("class_eval string", out, 7);
    } else {
        ok = 0;
    }
    if (eval_check(L, "class_eval block", "class CE2\n end\n class_eval(CE2) { define_method(\"bar\") { 9 } }\n CE2.new.bar", &out)) {
        ok &= assert_int("class_eval block", out, 9);
    } else {
        ok = 0;
    }
    if (eval_check(L, "instance_eval block", "class IE\n def val()\n 1\n end\n end\n o = IE.new\n instance_eval(o) { self.val() + 2 }", &out)) {
        ok &= assert_int("instance_eval block", out, 3);
    } else {
        ok = 0;
    }
    if (eval_check(L, "instance_eval string", "x = 0\n class IE2\n end\n o = IE2.new\n instance_eval(o, \"x = 5\")\n x", &out)) {
        ok &= assert_int("instance_eval string", out, 5);
    } else {
        ok = 0;
    }

    // singleton methods
    if (eval_check(L, "singleton method object", "class SO\n def foo()\n 1\n end\n end\n o = SO.new\n define_singleton_method(o, \"foo\") { 9 }\n o.foo", &out)) {
        ok &= assert_int("singleton method object", out, 9);
    } else {
        ok = 0;
    }
    if (eval_check(L, "singleton method class", "class SC\n end\n define_singleton_method(SC, \"bar\") { 7 }\n SC.bar", &out)) {
        ok &= assert_int("singleton method class", out, 7);
    } else {
        ok = 0;
    }

    // module inclusion order
    if (eval_check(L, "include order", "module MA\n def v()\n 1\n end\n end\n module MB\n def v()\n 2\n end\n end\n class MC\n include MA\n include MB\n end\n MC.new.v", &out)) {
        ok &= assert_int("include order", out, 2);
    } else {
        ok = 0;
    }
    if (eval_check(L, "include chain", "module M1\n def a()\n 1\n end\n end\n module M2\n include M1\n def a()\n 2\n end\n end\n class M3\n include M1\n include M2\n end\n M3.new.a", &out)) {
        ok &= assert_int("include chain", out, 2);
    } else {
        ok = 0;
    }

    if (eval_check(L, "prepend order", "module PM\n def v()\n 1\n end\n end\n class PC\n def v()\n 2\n end\n prepend PM\n end\n PC.new.v", &out)) {
        ok &= assert_int("prepend order", out, 1);
    } else {
        ok = 0;
    }

    if (eval_check(L, "prepend in module", "module PMA\n def v()\n 1\n end\n end\n module PMB\n def v()\n 2\n end\n end\n module PMC\n prepend PMA\n include PMB\n end\n class PCD\n include PMC\n end\n PCD.new.v", &out)) {
        ok &= assert_int("prepend in module", out, 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "prepend chain", "module P1\n def v()\n 1\n end\n end\n module P2\n def v()\n 2\n end\n end\n module P3\n prepend P1\n prepend P2\n end\n class P4\n include P3\n end\n P4.new.v", &out)) {
        ok &= assert_int("prepend chain", out, 2);
    } else {
        ok = 0;
    }

    // method_missing
    if (eval_check(L, "method_missing", "class C\n def method_missing(name)\n 42\n end\n end\n C.new.foo", &out)) {
        ok &= assert_int("method_missing", out, 42);
    } else {
        ok = 0;
    }

    // respond_to
    if (eval_check(L, "respond_to", "class D\n def bar()\n 1\n end\n end\n respond_to(D.new, \"bar\")", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }

    // proc params shouldn't clobber globals
    if (eval_check(L, "param scope", "x = 5; def foo(x)\n x + 1\n end\n foo(10); x", &out)) {
        ok &= assert_int("param scope", out, 5);
    } else {
        ok = 0;
    }

    // select truthiness
    if (eval_check(L, "select", "len([0,1,2,3].select { |x| x > 1 })", &out)) {
        ok &= assert_int("select", out, 2);
    } else {
        ok = 0;
    }

    // enumerable helpers
    if (eval_check(L, "reduce", "reduce([1,2,3,4], 0) { |acc, x| acc + x }", &out)) {
        ok &= assert_int("reduce", out, 10);
    } else {
        ok = 0;
    }
    if (eval_check(L, "any?", "any?([1,2,3]) { |x| x == 2 }", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "all?", "all?([1,2,3]) { |x| x > 0 }", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "none?", "none?([1,2,3]) { |x| x < 0 }", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "find", "find([1,2,3,4]) { |x| x > 2 }", &out)) {
        ok &= assert_int("find", out, 3);
    } else {
        ok = 0;
    }

    // return
    if (eval_check(L, "return", "def early()\n return 7\n 9\n end\n early()", &out)) {
        ok &= assert_int("return", out, 7);
    } else {
        ok = 0;
    }

    // unary ops
    if (eval_check(L, "unary not", "!false", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "unary neg", "-5", &out)) {
        ok &= assert_int("unary neg", out, -5);
    } else {
        ok = 0;
    }

    // safe navigation
    if (eval_check(L, "safe nav nil", "n = nil; if n&.foo()\n 1\n else\n 2\n end", &out)) {
        ok &= assert_int("safe nav nil", out, 2);
    } else {
        ok = 0;
    }
    if (eval_check(L, "safe nav call", "class SN\n def ok()\n 5\n end\n end\n t = SN.new\n t&.ok()", &out)) {
        ok &= assert_int("safe nav call", out, 5);
    } else {
        ok = 0;
    }
    if (eval_check(L, "safe nav args", "class SA\n def add(x)\n x + 1\n end\n end\n a = SA.new\n a&.add(2)", &out)) {
        ok &= assert_int("safe nav args", out, 3);
    } else {
        ok = 0;
    }
    if (eval_check(L, "safe nav index nil", "n = nil\n n&.[0]", &out)) {
        ok &= (out.type == LUBY_T_NIL);
    } else {
        ok = 0;
    }
    if (eval_check(L, "safe nav index array", "a = [10, 20, 30]\n a&.[1]", &out)) {
        ok &= assert_int("safe nav index array", out, 20);
    } else {
        ok = 0;
    }
    if (eval_check(L, "safe nav index hash", "h = { \"a\" => 7, \"b\" => 9 }\n h&.[\"b\"]", &out)) {
        ok &= assert_int("safe nav index hash", out, 9);
    } else {
        ok = 0;
    }
    if (eval_check(L, "safe nav index hash nil", "h = nil\n h&.[\"a\"]", &out)) {
        ok &= (out.type == LUBY_T_NIL);
    } else {
        ok = 0;
    }
    if (eval_check(L, "safe nav chain index", "a = [ [1,2], nil ]\n a&.[1]&.[0]", &out)) {
        ok &= (out.type == LUBY_T_NIL);
    } else {
        ok = 0;
    }
    if (eval_check(L, "safe nav chain index value", "a = [ [1,2], [3,4] ]\n a&.[1]&.[0]", &out)) {
        ok &= assert_int("safe nav chain index value", out, 3);
    } else {
        ok = 0;
    }
    if (eval_check(L, "safe nav chain call", "class SNC\n def arr()\n [10, 20]\n end\n end\n o = SNC.new\n o&.arr()&.[1]", &out)) {
        ok &= assert_int("safe nav chain call", out, 20);
    } else {
        ok = 0;
    }
    if (eval_check(L, "safe nav chain call nil", "n = nil\n n&.foo()&.[0]", &out)) {
        ok &= (out.type == LUBY_T_NIL);
    } else {
        ok = 0;
    }

    // dig
    if (eval_check(L, "dig array", "a = [ [1,2], [3, [4]] ]\n dig(a, 1, 1, 0)", &out)) {
        ok &= assert_int("dig array", out, 4);
    } else {
        ok = 0;
    }
    if (eval_check(L, "dig hash", "h = { \"a\" => { \"b\" => 2 } }\n dig(h, \"a\", \"b\")", &out)) {
        ok &= assert_int("dig hash", out, 2);
    } else {
        ok = 0;
    }
    if (eval_check(L, "dig mixed", "h = { \"a\" => [ { \"b\" => 5 } ] }\n dig(h, \"a\", 0, \"b\")", &out)) {
        ok &= assert_int("dig mixed", out, 5);
    } else {
        ok = 0;
    }
    if (eval_check(L, "dig missing", "h = { \"a\" => { \"b\" => 2 } }\n dig(h, \"a\", \"c\")", &out)) {
        ok &= (out.type == LUBY_T_NIL);
    } else {
        ok = 0;
    }

    // frozen
    if (eval_check(L, "frozen array", "a = [1,2]\n freeze(a)\n frozen?(a)", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "frozen hash", "h = { \"a\" => 1 }\n freeze(h)\n frozen?(h)", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "frozen nil", "frozen?(nil)", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "frozen object", "class FZ\n end\n o = FZ.new\n freeze(o)\n frozen?(o)", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "frozen class", "class FC\n end\n freeze(FC)\n frozen?(FC)", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }

    // and/or
    if (eval_check(L, "and", "(1 < 2) && (2 < 3)", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "or", "(1 > 2) || (2 < 3)", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }

    // comparisons
    if (eval_check(L, "cmp eq", "1 == 1", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "cmp lt", "1 < 2", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }
    if (eval_check(L, "cmp gte", "2 >= 2", &out)) {
        ok &= (out.type == LUBY_T_BOOL && out.as.b == 1);
    } else {
        ok = 0;
    }

    // algorithms
    if (eval_check(L, "factorial", "def fact(n)\n if n <= 1\n  1\n else\n  n * fact(n - 1)\n end\n end\n fact(6)", &out)) {
        ok &= assert_int("factorial", out, 720);
    } else {
        ok = 0;
    }

    if (eval_check(L, "fibonacci", "def fib(n)\n if n <= 1\n  n\n else\n  fib(n - 1) + fib(n - 2)\n end\n end\n fib(10)", &out)) {
        ok &= assert_int("fibonacci", out, 55);
    } else {
        ok = 0;
    }

    if (eval_check(L, "gcd", "def gcd(a, b)\n while b > 0\n  t = b\n  b = a % b\n  a = t\n end\n a\n end\n gcd(48, 18)", &out)) {
        ok &= assert_int("gcd", out, 6);
    } else {
        ok = 0;
    }

    if (eval_check(L, "prime sieve", "n = 30\n sieve = []\n i = 0\n while i <= n\n  sieve[i] = 1\n  i = i + 1\n end\n sieve[0] = 0\n sieve[1] = 0\n p = 2\n while p * p <= n\n  if sieve[p] == 1\n   j = p * p\n   while j <= n\n    sieve[j] = 0\n    j = j + p\n   end\n  end\n  p = p + 1\n end\n count = 0\n i = 2\n while i <= n\n  if sieve[i] == 1\n   count = count + 1\n  end\n  i = i + 1\n end\n count", &out)) {
        ok &= assert_int("prime sieve", out, 10);
    } else {
        ok = 0;
    }

    if (eval_check(L, "insertion sort", "def isort(a)\n i = 1\n while i < len(a)\n  key = a[i]\n  j = i - 1\n  while j >= 0 && a[j] > key\n   a[j + 1] = a[j]\n   j = j - 1\n  end\n  a[j + 1] = key\n  i = i + 1\n end\n a\n end\n a = [5,3,4,1,2]\n b = isort(a)\n b[0] + b[1] + b[2] + b[3] + b[4]", &out)) {
        ok &= assert_int("insertion sort", out, 15);
    } else {
        ok = 0;
    }

    if (eval_check(L, "binary search", "def bsearch(a, t)\n lo = 0\n hi = len(a) - 1\n while lo <= hi\n  mid = (lo + hi) / 2\n  if a[mid] == t\n   return mid\n  end\n  if a[mid] < t\n   lo = mid + 1\n  else\n   hi = mid - 1\n  end\n end\n -1\n end\n a = [1,3,5,7,9]\n bsearch(a, 7)", &out)) {
        ok &= assert_int("binary search", out, 3);
    } else {
        ok = 0;
    }

    if (eval_check(L, "bfs grid", "def bfs(grid)\n h = len(grid)\n w = len(grid[0])\n qx = []\n qy = []\n head = 0\n tail = 0\n qx[tail] = 0\n qy[tail] = 0\n tail = tail + 1\n dist = []\n i = 0\n while i < h\n  dist[i] = []\n  j = 0\n  while j < w\n   dist[i][j] = -1\n   j = j + 1\n  end\n  i = i + 1\n end\n dist[0][0] = 0\n while head < tail\n  x = qx[head]\n  y = qy[head]\n  head = head + 1\n  d = dist[y][x]\n  if x == w - 1 && y == h - 1\n   return d\n  end\n  nx = x + 1\n  ny = y\n  if nx < w && dist[ny][nx] < 0 && grid[ny][nx] == 0\n   dist[ny][nx] = d + 1\n   qx[tail] = nx\n   qy[tail] = ny\n   tail = tail + 1\n  end\n  nx = x - 1\n  ny = y\n  if nx >= 0 && dist[ny][nx] < 0 && grid[ny][nx] == 0\n   dist[ny][nx] = d + 1\n   qx[tail] = nx\n   qy[tail] = ny\n   tail = tail + 1\n  end\n  nx = x\n  ny = y + 1\n  if ny < h && dist[ny][nx] < 0 && grid[ny][nx] == 0\n   dist[ny][nx] = d + 1\n   qx[tail] = nx\n   qy[tail] = ny\n   tail = tail + 1\n  end\n  nx = x\n  ny = y - 1\n  if ny >= 0 && dist[ny][nx] < 0 && grid[ny][nx] == 0\n   dist[ny][nx] = d + 1\n   qx[tail] = nx\n   qy[tail] = ny\n   tail = tail + 1\n  end\n end\n -1\n end\n g = [[0,0,0],[1,1,0],[0,0,0]]\n bfs(g)", &out)) {
        ok &= assert_int("bfs grid", out, 4);
    } else {
        ok = 0;
    }

    if (eval_check(L, "stack", "s = []\n array_push(s, 1)\n array_push(s, 2)\n array_push(s, 3)\n a = array_pop(s)\n b = array_pop(s)\n c = array_pop(s)\n a * 100 + b * 10 + c", &out)) {
        ok &= assert_int("stack", out, 321);
    } else {
        ok = 0;
    }

    if (eval_check(L, "queue", "q = []\n head = 0\n array_push(q, 1)\n array_push(q, 2)\n array_push(q, 3)\n a = q[head]\n head = head + 1\n b = q[head]\n head = head + 1\n c = q[head]\n a * 100 + b * 10 + c", &out)) {
        ok &= assert_int("queue", out, 123);
    } else {
        ok = 0;
    }

    if (eval_check(L, "min heap", "def hpush(h, v)\n array_push(h, v)\n i = len(h) - 1\n while i > 0\n  p = (i - 1) / 2\n  if h[p] <= h[i]\n   break\n  end\n  t = h[p]\n  h[p] = h[i]\n  h[i] = t\n  i = p\n end\n end\n def hpop(h)\n if len(h) == 0\n  return -1\n end\n root = h[0]\n last = array_pop(h)\n if len(h) > 0\n  h[0] = last\n  i = 0\n  while true\n   l = i * 2 + 1\n   r = i * 2 + 2\n   if l >= len(h)\n    break\n   end\n   s = l\n   if r < len(h) && h[r] < h[l]\n    s = r\n   end\n   if h[i] <= h[s]\n    break\n   end\n   t = h[i]\n   h[i] = h[s]\n   h[s] = t\n   i = s\n  end\n end\n root\n end\n h = []\n hpush(h, 5)\n hpush(h, 3)\n hpush(h, 4)\n hpush(h, 1)\n hpush(h, 2)\n a = hpop(h)\n b = hpop(h)\n c = hpop(h)\n d = hpop(h)\n e = hpop(h)\n a * 10000 + b * 1000 + c * 100 + d * 10 + e", &out)) {
        ok &= assert_int("min heap", out, 12345);
    } else {
        ok = 0;
    }

    if (eval_check(L, "quicksort", "def qsort(a, lo, hi)\n if lo >= hi\n  return a\n end\n i = lo\n j = hi\n pivot = a[(lo + hi) / 2]\n while i <= j\n  while a[i] < pivot\n   i = i + 1\n  end\n  while a[j] > pivot\n   j = j - 1\n  end\n  if i <= j\n   t = a[i]\n   a[i] = a[j]\n   a[j] = t\n   i = i + 1\n   j = j - 1\n  end\n end\n if lo < j\n  qsort(a, lo, j)\n end\n if i < hi\n  qsort(a, i, hi)\n end\n a\n end\n a = [9,7,5,3,1,2,4,6,8]\n qsort(a, 0, len(a) - 1)\n a[0] + a[1] + a[2] + a[3] + a[4] + a[5] + a[6] + a[7] + a[8]", &out)) {
        ok &= assert_int("quicksort", out, 45);
    } else {
        ok = 0;
    }

    if (eval_check(L, "lcs", "def lcs(a, b)\n n = len(a)\n m = len(b)\n dp = []\n i = 0\n while i <= n\n  dp[i] = []\n  j = 0\n  while j <= m\n   dp[i][j] = 0\n   j = j + 1\n  end\n  i = i + 1\n end\n i = 1\n while i <= n\n  j = 1\n  while j <= m\n   if a[i - 1] == b[j - 1]\n    dp[i][j] = dp[i - 1][j - 1] + 1\n   else\n    if dp[i - 1][j] > dp[i][j - 1]\n     dp[i][j] = dp[i - 1][j]\n    else\n     dp[i][j] = dp[i][j - 1]\n    end\n   end\n   j = j + 1\n  end\n  i = i + 1\n end\n dp[n][m]\n end\n a = [1,2,3,2,4,1,2]\n b = [2,4,3,1,2,1]\n lcs(a, b)", &out)) {
        ok &= assert_int("lcs", out, 4);
    } else {
        ok = 0;
    }

    if (eval_check(L, "dijkstra", "def dijkstra(n, edges, src)\n inf = 999999\n dist = []\n used = []\n i = 0\n while i < n\n  dist[i] = inf\n  used[i] = 0\n  i = i + 1\n end\n dist[src] = 0\n i = 0\n while i < n\n  v = -1\n  j = 0\n  while j < n\n   if used[j] == 0 && (v == -1 || dist[j] < dist[v])\n    v = j\n   end\n   j = j + 1\n  end\n  if v == -1\n   break\n  end\n  used[v] = 1\n  j = 0\n  while j < len(edges)\n   e = edges[j]\n   u = e[0]\n   to = e[1]\n   w = e[2]\n   if u == v && dist[v] + w < dist[to]\n    dist[to] = dist[v] + w\n   end\n   j = j + 1\n  end\n  i = i + 1\n end\n dist\n end\n edges = [[0,1,4],[0,2,1],[2,1,2],[1,3,1],[2,3,5]]\n d = dijkstra(4, edges, 0)\n d[3]", &out)) {
        ok &= assert_int("dijkstra", out, 4);
    } else {
        ok = 0;
    }

    // String interpolation tests
    if (eval_check(L, "interp simple", "name = \"world\"; \"Hello #{name}!\"", &out)) {
        ok &= assert_string("interp simple", out, "Hello world!");
    } else {
        ok = 0;
    }

    if (eval_check(L, "interp int", "x = 42; \"The answer is #{x}\"", &out)) {
        ok &= assert_string("interp int", out, "The answer is 42");
    } else {
        ok = 0;
    }

    if (eval_check(L, "interp expr", "a = 3; b = 4; \"#{a} + #{b} = #{a + b}\"", &out)) {
        ok &= assert_string("interp expr", out, "3 + 4 = 7");
    } else {
        ok = 0;
    }

    if (eval_check(L, "interp nested braces", "h = {\"x\" => 10}; \"value: #{h[\"x\"]}\"", &out)) {
        ok &= assert_string("interp nested braces", out, "value: 10");
    } else {
        ok = 0;
    }

    if (eval_check(L, "no interp single quote", "'Hello #{name}'", &out)) {
        ok &= assert_string("no interp single quote", out, "Hello #{name}");
    } else {
        ok = 0;
    }

    // luby_define_class / luby_define_method tests
    {
        luby_class *calc = luby_define_class(L, "NativeCalc", NULL);
        if (!calc) {
            printf("FAIL: luby_define_class returned NULL\n");
            ok = 0;
        } else {
            luby_define_method(L, calc, "double", native_double);
            luby_define_method(L, calc, "add", native_add);

            if (eval_check(L, "native method double", "c = NativeCalc.new; c.double(21)", &out)) {
                ok &= assert_int("native method double", out, 42);
            } else {
                ok = 0;
            }

            if (eval_check(L, "native method add", "c = NativeCalc.new; c.add(10, 32)", &out)) {
                ok &= assert_int("native method add", out, 42);
            } else {
                ok = 0;
            }
        }
    }

    // Singleton method tests
    if (eval_check(L, "singleton method on object", "class Point; end\np = Point.new\ndef p.x\n  42\nend\np.x", &out)) {
        ok &= assert_int("singleton method on object", out, 42);
    } else {
        ok = 0;
    }

    // def self.method inside a class - adds singleton method to the class itself
    if (eval_check(L, "class method def self", "class Counter\n  def self.count\n    99\n  end\nend\nCounter.count", &out)) {
        ok &= assert_int("class method def self", out, 99);
    } else {
        ok = 0;
    }

    // Test luby_invoke_global for Luby-defined functions
    {
        // Define a function in Luby
        luby_value discard;
        const char *code1 = "def add_three(x)\n  x + 3\nend";
        if (luby_eval(L, code1, 0, "<test>", &discard) == 0) {
            // Call it from C
            luby_value args[1] = { luby_int(10) };
            luby_value result;
            if (luby_invoke_global(L, "add_three", 1, args, &result) == 0) {
                ok &= assert_int("luby_invoke_global", result, 13);
            } else {
                printf("FAIL: luby_invoke_global returned error\n");
                ok = 0;
            }
        } else {
            printf("FAIL: could not define add_three\n");
            ok = 0;
        }
    }

    // Test luby_invoke_method for Luby-defined methods
    {
        luby_value discard;
        // Define class and create instance in one go
        const char *code2 = "class InvokeAdder\n  def add(a, b)\n    a + b\n  end\nend\ntest_adder = InvokeAdder.new";
        int rc2 = luby_eval(L, code2, 0, "<test>", &discard);
        if (rc2 != 0) {
            char buf[256];
            luby_format_error(L, buf, sizeof(buf));
            printf("FAIL: could not define InvokeAdder class: %s\n", buf);
            ok = 0;
        } else {
            // Get the instance
            luby_string_view sv = { "test_adder", 10 };
            luby_value obj = luby_get_global(L, sv);
            if (obj.type == LUBY_T_OBJECT) {
                luby_value args[2] = { luby_int(7), luby_int(8) };
                luby_value result;
                if (luby_invoke_method(L, obj, "add", 2, args, &result) == 0) {
                    ok &= assert_int("luby_invoke_method", result, 15);
                } else {
                    printf("FAIL: luby_invoke_method returned error\n");
                    ok = 0;
                }
            } else {
                printf("FAIL: test_adder not an object (type=%d)\n", obj.type);
                ok = 0;
            }
        }
    }

    luby_free(L);
    return ok ? 0 : 1;
}
