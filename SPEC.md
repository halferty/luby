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
