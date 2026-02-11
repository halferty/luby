# Luby TODO

## In Progress
- [ ] Garbage collector (mark-and-sweep or refcounting)

## Planned
- [ ] Regex support
- [ ] String interpolation improvements
- [ ] Enumerator / lazy enumerator support
- [ ] Range type (`1..10`, `1...10`)
- [ ] Splat args (`*args`) and keyword args
- [ ] `lambda` vs `proc` distinction
- [ ] `Comparable` and `Enumerable` as includable modules
- [ ] Arena allocation for short-lived eval chunks
- [ ] Weak references for host-owned userdata
- [ ] Fiber support

## Done
- [x] Lexer and parser
- [x] Bytecode compiler and VM
- [x] Core types (Integer, Float, String, Symbol, Array, Hash, Proc)
- [x] Classes, modules, inheritance, mixins
- [x] Singleton methods and metaclasses
- [x] Metaprogramming (`method_missing`, `define_method`, `send`, `class_eval`, `instance_eval`)
- [x] Blocks, `yield`, Enumerable methods
- [x] Visibility (`public`, `private`, `protected`)
- [x] Exception handling (`begin`/`rescue`/`ensure`/`raise`)
- [x] Coroutines (yield/resume from host)
- [x] VFS-based `require`/`load`
- [x] Debug hooks (line, call, return)
- [x] Custom allocator support
- [x] Userdata with finalizers
- [x] Native function and module registration
- [x] File/line error reporting
