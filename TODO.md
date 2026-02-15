# Luby TODO

## In Progress

## Planned
- [ ] Bytecode caching
- [ ] Vector2, Vector3, Quaternion, Matrix, Color types
- [ ] Seeded random, gaussian/uniform distributions, shuffle, sample, rand
- [ ] Easing/interpolation cubic/elastic/bounce
- [ ] schedule/schedule_repeating run methods every N seconds, frame-based and time-based variants
- [ ] coroutine enhancements - wait(sec) yield until time passes, wait_until { condition } yield until condition true, wait_frames(n) yield for N frames
- [ ] set type
- [ ] piority queue
- [ ] object pooling
- [ ] binary pack/unpack
- [ ] json support
- [ ] EventEmitter mixin or class, on(event, &handler), emit(event, data)
- [ ] state machine helper
- [ ] ECS pattern helpers
- [ ] profiling hooks, function call counts, execution time per function, memory allocation tracking
- [ ] Local variable inspection on stack traces, conditional breakpoints
- [ ] Hot reloading, game state preserves across reloads


## Maybe Later
- [p] Regex support (PUNT)

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
- [x] Lambda (`->`) syntax (note: no strict arity distinction from proc)
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
- [x] `Object#inspect` - string representation suitable for debugging (shows strings with quotes, symbols with colons, objects with class and memory address)
- [x] `Object#object_id` - unique identifier for each object (immediate values get deterministic IDs, heap objects use pointer address)
- [x] `Class#name`, `Class#superclass`, `Class#ancestors` - class introspection methods
- [x] `Symbol#to_sym` - returns self (a symbol is already a symbol)
- [x] `String#to_f` - convert string to float (already implemented via base `to_f`)
- [x] String + non-string auto-stringifies and concatenates (`"hello" + 42` → `"hello42"`)
- [x] Integer/float division and modulo by zero raises `ZeroDivisionError`
- [x] Inheriting from an undefined class raises `NameError`
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
- [x] Execution limits — 4 limit types for safe game scripting: instruction limit (per-invocation), call depth limit (stack overflow protection), allocation count limit (per-invocation), memory limit (persistent GC heap cap). Counters reset on each C→Ruby entry (`luby_eval`, `coroutine_resume`). Configurable via `luby_config` or dynamic API (`luby_set_instruction_limit`, `luby_set_call_depth_limit`, `luby_set_allocation_limit`, `luby_set_memory_limit`). Query functions: `luby_get_instruction_count`, `luby_get_allocation_count`, `luby_get_memory_usage`, `luby_get_peak_memory_usage`. Limits of 0 mean unlimited (backward compatible).