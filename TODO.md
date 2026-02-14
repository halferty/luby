# Luby TODO

## In Progress

## Planned

## Maybe Later
- [ ] Regex support

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
- [x] Implicit self method calls (calling methods without explicit receiver resolves to `self.method`)
- [x] Heredoc strings (`<<IDENT`, `<<-IDENT`, `<<"IDENT"`)
- [x] `case`/`when` expressions
- [x] `retry` in exception handling
- [x] `__method__` and `__callee__`
- [x] `caller` method for stack introspection
- [x] `loop` keyword (`loop do ... end`)
- [x] Arena allocation for AST nodes (16KB block-based bump allocator, freed after compilation)
- [x] Invalidatable userdata (VM-owned or wrapped host pointers, class dispatch, finalizers, tombstone on invalidation)
- [x] `break`/`next` with values (`break 42`, `break value if cond`)
- [x] `for` loops (`for x in collection do ... end`) with optional `do` keyword
- [x] `while`/`until` with optional `do` keyword (`while cond do ... end`)
- [x] `break`/`next` inside block iterators (`each`, `map`, `select`, `times`, `reduce`, etc.)
- [x] Lexer fix: integer literal followed by dot-method (`3.times`) no longer misparsed as float
- [x] `Struct` class (`Struct.new(:name, :age)`) with generated `initialize`, accessors, `to_a`, `to_h`, `members`, `==`, `[]`, `[]=`, `each`, `to_s`
- [x] `Comparable` module (`<`, `<=`, `==`, `>`, `>=`, `between?`, `clamp` via `<=>`)
- [x] `Enumerable` module (30 methods: `to_a`, `map`, `select`, `reject`, `find`, `count`, `include?`, `min`, `max`, `sum`, `reduce`, `any?`, `all?`, `none?`, `min_by`, `max_by`, `sort`, `sort_by`, `flat_map`, `each_with_index`, `first`, `take`, `drop`, `group_by`, `tally`, `zip`, `each_with_object`, `entries`, `collect`)
- [x] Spaceship operator (`<=>`) with built-in handling for Integer, Float, String
- [x] Operator method definitions (`def <=>`, `def []`, `def []=`, setter methods `def name=`)
- [x] Object comparison dispatch via `<=>` in comparison opcodes
- [x] GC fix: pause GC during compilation (compiled procs in chunk constants are not GC roots)
- [x] `Fiber` class (`Fiber.new { }`, `fiber.resume(val)`, `Fiber.yield(val)`, `fiber.alive?`) — cooperative concurrency with bidirectional value passing, built on existing coroutine infrastructure via `luby_native_yield`
- [x] Lazy enumerator (`[1,2,3].lazy`, `(1..100).lazy`) — chain-based pipeline with `map`, `select`, `reject`, `take`, `drop`, `flat_map`, `first`, and all consuming methods; short-circuits for `take`/`drop`/`first`