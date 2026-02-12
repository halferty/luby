# Luby TODO

## In Progress

## Planned
- [ ] Regex support
- [ ] `Comparable` and `Enumerable` as includable modules
- [ ] Lazy enumerator support
- [ ] Arena allocation for short-lived eval chunks
- [ ] Weak references for host-owned userdata
- [ ] Fiber support
- [ ] Heredoc strings
- [ ] Implicit self method calls (calling methods without explicit receiver from within methods)

## Done
- [x] Garbage collector (mark-and-sweep with intrusive linked list)
- [x] Lexer and parser
- [x] Bytecode compiler and VM
- [x] Core types (Integer, Float, String, Symbol, Array, Hash, Proc, Range)
- [x] Classes, modules, inheritance, mixins (`include`, `prepend`, `extend`)
- [x] Singleton methods and metaclasses
- [x] Metaprogramming (`method_missing`, `define_method`, `send`, `class_eval`, `instance_eval`)
- [x] Blocks, `yield`, Enumerable methods (`map`, `select`, `reduce`, `each_with_index`, etc.)
- [x] Visibility (`public`, `private`, `protected`)
- [x] Exception handling (`begin`/`rescue`/`ensure`/`raise`)
- [x] Coroutines (yield/resume from host)
- [x] VFS-based `require`/`load`
- [x] Debug hooks (line, call, return)
- [x] Custom allocator support
- [x] Userdata with finalizers
- [x] Native function and module registration
- [x] File/line error reporting
- [x] `initialize` method called by `new`
- [x] Range type (`1..10`, `1...10`) with `to_a`, `each`, `include?`, `size`
- [x] Splat args (`*args`) and keyword args (`name:`, `name: default`)
- [x] Lambda (`->`) syntax (note: no strict arity distinction from proc yet)
- [x] String interpolation (`"Hello, #{name}!"`)
- [x] Enumerator class (basic support for `each_with_index` without block)
- [x] Symbol-to-proc (`&:method_name`)
- [x] Conditional assignment (`||=`, `&&=`)
- [x] Compound assignment (`+=`, `-=`, `*=`, `/=`, `%=`)
- [x] Multi-assignment (`a, b = 1, 2` and `a, b = [1, 2]`)
- [x] Ternary operator (`cond ? then : else`)
- [x] Safe navigation operator (`&.`)
- [x] Statement modifiers (`x = 5 if cond`, `x = 5 unless cond`)
- [x] `alias` for method aliasing
- [x] `attr_reader`, `attr_writer`, `attr_accessor`
- [x] Numeric predicates (`zero?`, `positive?`, `negative?`, `even?`, `odd?`)
- [x] Reflection (`is_a?`, `kind_of?`, `instance_of?`, `respond_to?`, `defined?`)
- [x] Class variables (`@@var`)
- [x] `module_function` (makes methods both private instance methods and public singleton methods)
