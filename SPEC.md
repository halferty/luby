# Luby Language Guide

Luby implements a practical subset of Ruby. This document describes what the language supports.

---

## Types

| Type | Examples |
|------|---------|
| Integer | `42`, `-7`, `0xFF` |
| Float | `3.14`, `-0.5` |
| String | `"hello"`, `'world'` |
| Symbol | `:name`, `:ok` |
| Array | `[1, 2, 3]` |
| Hash | `{ a: 1, "b" => 2 }` |
| Boolean | `true`, `false` |
| Nil | `nil` |
| Proc | `proc { |x| x + 1 }` |

Everything is an object, including classes and modules.

---

## Variables & Assignment

```ruby
x = 10
name = "Luby"
```

## Control Flow

```ruby
# if / elsif / else
if x > 0
  puts "positive"
elsif x == 0
  puts "zero"
else
  puts "negative"
end

# unless
unless done
  work
end

# while / until
while alive
  fight
end

until empty
  pop
end

# case / when
case status
when :ok
  proceed
when :error
  abort
end

# for (desugars to each)
for x in [1, 2, 3]
  puts x
end
```

`break`, `next`, `redo`, and `return` are supported inside loops and methods.

---

## Methods

```ruby
def greet(name)
  "Hello, #{name}!"
end

puts greet("world")
```

Methods return the value of their last expression.

---

## Blocks & Iterators

```ruby
[1, 2, 3].each { |n| puts n }

[1, 2, 3].map { |n| n * 2 }        #=> [2, 4, 6]
[1, 2, 3].select { |n| n > 1 }     #=> [2, 3]
[1, 2, 3].reject { |n| n.even? }
[1, 2, 3].reduce(0) { |sum, n| sum + n }

[1, 2, 3].any? { |n| n > 2 }       #=> true
[1, 2, 3].all? { |n| n > 0 }       #=> true
[1, 2, 3].none? { |n| n > 5 }      #=> true
[1, 2, 3].find { |n| n.even? }     #=> 2
```

Blocks are closures. Methods accept a block with `yield`:

```ruby
def twice
  yield
  yield
end

twice { puts "hey" }
```

`Proc` objects are first-class:

```ruby
double = proc { |x| x * 2 }
double.call(5)  #=> 10
```

---

## Classes & Inheritance

```ruby
class Animal
  def initialize(name)
    @name = name
  end

  def speak
    "..."
  end
end

class Dog < Animal
  def speak
    "Woof! I'm #{@name}"
  end
end

Dog.new("Rex").speak  #=> "Woof! I'm Rex"
```

`super` calls the parent implementation.

### Visibility

Methods can be declared `public`, `private`, or `protected`:

```ruby
class Secrets
  def visible
    hidden
  end

  private

  def hidden
    "shh"
  end
end
```

---

## Modules & Mixins

```ruby
module Greetable
  def greet
    "Hi, I'm #{@name}"
  end
end

class Person
  include Greetable

  def initialize(name)
    @name = name
  end
end

Person.new("Alice").greet  #=> "Hi, I'm Alice"
```

The `included` hook is called when a module is mixed in.

---

## Struct

```ruby
Point = Struct.new(:x, :y)
p = Point.new(10, 20)
p.x            #=> 10
p.to_a         #=> [10, 20]
p.to_h         #=> { x: 10, y: 20 }
p.members      #=> [:x, :y]
p == Point.new(10, 20)  #=> true
```

Struct classes automatically get `initialize`, readers, writers, `[]`, `[]=`, `each`, `to_s`, `==`, `to_a`, `to_h`, and `members`.

---

## Comparable

Include `Comparable` and define `<=>` to get `<`, `<=`, `==`, `>`, `>=`, `between?`, and `clamp`:

```ruby
class Temperature
  include Comparable
  attr_reader :deg
  def initialize(d); @deg = d; end
  def <=>(other); @deg <=> other.deg; end
end

Temperature.new(50) > Temperature.new(30)   #=> true
Temperature.new(10).between?(Temperature.new(0), Temperature.new(20))  #=> true
```

---

## Enumerable

Include `Enumerable` and define `each` to get 30 iteration methods:

```ruby
class NumberList
  include Enumerable
  def initialize(*nums); @nums = nums; end
  def each(&block); @nums.each(&block); end
end

list = NumberList.new(3, 1, 2)
list.sort          #=> [1, 2, 3]
list.map { |n| n * 2 }  #=> [6, 2, 4]
list.min           #=> 1
list.any? { |n| n > 2 } #=> true
```

Methods include: `to_a`, `map`/`collect`, `select`, `reject`, `find`, `count`, `include?`, `min`, `max`, `sum`, `reduce`, `any?`, `all?`, `none?`, `min_by`, `max_by`, `sort`, `sort_by`, `flat_map`, `each_with_index`, `first`, `take`, `drop`, `group_by`, `tally`, `zip`, `each_with_object`, `entries`.

---

## Singleton Methods

```ruby
obj = Object.new
def obj.hello
  "hi from singleton"
end
obj.hello  #=> "hi from singleton"
```

---

## Metaprogramming

```ruby
# method_missing — catch undefined calls
class Flexible
  def method_missing(name, *args)
    "You called #{name}"
  end

  def respond_to_missing?(name, include_private = false)
    true
  end
end

# define_method — create methods at runtime
class Builder
  ["width", "height"].each do |attr|
    define_method(attr) { instance_variable_get("@#{attr}") }
  end
end

# send — dynamic dispatch
obj.send(:speak)

# class_eval / instance_eval
String.class_eval do
  def shout
    upcase + "!"
  end
end
```

### Hooks

| Hook | Called when… |
|------|-------------|
| `method_missing` | A method is not found |
| `respond_to_missing?` | `respond_to?` is queried for a missing method |
| `inherited` | A class is subclassed |
| `included` | A module is included |

---

## Exceptions

```ruby
begin
  raise "oops"
rescue => e
  puts e.message
ensure
  cleanup
end
```

---

## Fibers

Fibers provide cooperative concurrency — a fiber runs until it yields, then control returns to the caller.

```ruby
f = Fiber.new { |x|
  Fiber.yield(x + 1)
  x + 2
}

f.resume(10)  #=> 11  (yielded value)
f.resume      #=> 12  (return value)
f.alive?      #=> false
```

Bidirectional value passing:

```ruby
f = Fiber.new {
  val = Fiber.yield(1)   # yield 1, receive value from next resume
  val * 10
}
f.resume        #=> 1
f.resume(5)     #=> 50
```

Infinite generators:

```ruby
fib = Fiber.new {
  a, b = 0, 1
  loop do
    Fiber.yield(a)
    a, b = b, a + b
  end
}

10.times { puts fib.resume }  # prints first 10 Fibonacci numbers
```

The `yield` keyword also works inside fibers (equivalent to `Fiber.yield`).

---

## Lazy Enumerators

Call `.lazy` on an Array or Range (or any Enumerable) to get a `Lazy` object that chains transforms without evaluating them until forced:

```ruby
# Build a pipeline — nothing runs yet
lazy = (1..1000000).lazy.select { |x| x % 2 == 0 }.map { |x| x * 10 }.take(5)

lazy.to_a    #=> [20, 40, 60, 80, 100]   — only 10 elements examined
lazy.force   #=> same as to_a
```

### Chainable methods

| Method | Description |
|--------|-------------|
| `map { \|x\| ... }` / `collect` | Transform each element |
| `select { \|x\| ... }` / `filter` | Keep elements where block is truthy |
| `reject { \|x\| ... }` | Drop elements where block is truthy |
| `take(n)` | Stop after *n* elements (short-circuits) |
| `drop(n)` | Skip the first *n* elements |
| `flat_map { \|x\| ... }` | Map then flatten one level |
| `lazy` | Returns self (no-op on Lazy) |

### Forcing / consuming

| Method | Description |
|--------|-------------|
| `to_a` / `force` | Materialize the pipeline into an Array |
| `each { \|x\| ... }` | Iterate the pipeline, yielding to the block |
| `first` / `first(n)` | First element or first *n* elements |
| `count`, `sum`, `min`, `max` | Aggregate the pipeline |
| `reduce(init) { \|acc, x\| ... }` | Fold |
| `find { \|x\| ... }` | First matching element |
| `any?`, `all?`, `none?`, `include?` | Boolean queries |

### Custom Enumerable classes

Any class that includes `Enumerable` and defines `each` automatically gets a `.lazy` method:

```ruby
class Evens
  include Enumerable
  def each
    n = 0
    loop { yield n; n = n + 2 }
  end
end

# Evens.new.lazy.take(5).to_a  #=> [0, 2, 4, 6, 8]
```

---

## Range

```ruby
(1..5).to_a       #=> [1, 2, 3, 4, 5]
(1...5).to_a      #=> [1, 2, 3, 4]
(1..5).include?(3) #=> true
(1..10).size       #=> 10
(1..5).each { |n| puts n }
```

`..` is inclusive, `...` is exclusive of the end value.

---

## File Loading

```ruby
require "utils"
load "config.rb"
```

File access is delegated to the host application's virtual file system. See [EMBEDDING.md](EMBEDDING.md) for how to set this up.

---

## Method Lookup Order

1. Eigenclass (singleton class)
2. Object's class
3. Included modules (last included wins)
4. Superclass chain
5. `method_missing`

---

## Standard Methods

A non-exhaustive list of built-in methods available after `luby_open_base`:

### Kernel
`puts`, `print`, `p`, `raise`, `require`, `load`

### Integer / Float
`+`, `-`, `*`, `/`, `%`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `to_s`, `to_i`, `to_f`, `even?`, `odd?`, `abs`, `times`

### String
`+`, `*`, `length`, `upcase`, `downcase`, `include?`, `split`, `strip`, `to_i`, `to_f`, `to_s`, `[]`

### Symbol
`to_s`, `to_sym`

### Array
`[]`, `[]=`, `push`, `pop`, `length`, `size`, `each`, `map`, `select`, `reject`, `reduce`, `inject`, `any?`, `all?`, `none?`, `find`, `include?`, `flatten`, `compact`, `sort`, `reverse`, `first`, `last`, `empty?`, `join`

### Hash
`[]`, `[]=`, `keys`, `values`, `each`, `length`, `size`, `has_key?`, `has_value?`, `merge`, `delete`

### Object
`class`, `is_a?`, `respond_to?`, `send`, `nil?`, `to_s`, `inspect`, `object_id`, `freeze`, `frozen?`

### Class / Module
`new`, `name`, `superclass`, `define_method`, `class_eval`, `instance_eval`, `include`, `ancestors`

### Struct
`new`, `members`, `to_a`, `to_h`, `[]`, `[]=`, `each`, `==`, `to_s`, plus generated reader/writer methods

### Comparable (module)
`<`, `<=`, `==`, `>`, `>=`, `between?`, `clamp` — requires `<=>` to be defined

### Enumerable (module)
`to_a`, `map`, `select`, `reject`, `find`, `count`, `include?`, `min`, `max`, `sum`, `reduce`, `any?`, `all?`, `none?`, `min_by`, `max_by`, `sort`, `sort_by`, `flat_map`, `each_with_index`, `first`, `take`, `drop`, `group_by`, `tally`, `zip`, `each_with_object`, `entries`, `collect` — requires `each` to be defined

### Fiber
`Fiber.new { }`, `Fiber.yield(val)`, `fiber.resume(val)`, `fiber.alive?`

### Lazy (class)
`map`, `collect`, `select`, `filter`, `reject`, `take`, `drop`, `flat_map`, `to_a`, `force`, `each`, `first`, `include?`, `any?`, `all?`, `none?`, `count`, `min`, `max`, `sum`, `reduce`, `find`, `lazy` — created via `.lazy` on Array, Range, or any Enumerable
