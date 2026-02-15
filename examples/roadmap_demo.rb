#!/usr/bin/env ruby
# Demo of the newly implemented features

puts "=== Object#inspect ==="
str = "hello\nworld"
puts "String: " + str.inspect
sym = :test
puts "Symbol: " + sym.inspect
num = 42
puts "Integer: " + num.inspect
flt = 3.14
puts "Float: " + flt.inspect
puts "Nil: " + nil.inspect
puts "True: " + true.inspect
puts "False: " + false.inspect

puts ""
puts "=== Object#object_id ==="
print "nil.object_id: "
puts nil.object_id
print "true.object_id: "
puts true.object_id
print "false.object_id: "
puts false.object_id
print "5.object_id: "
puts 5.object_id
print "10.object_id: "
puts 10.object_id

puts ""
puts "=== Class#name, #superclass, #ancestors ==="
class Animal
end

class Dog < Animal
end

class Cat < Animal
end

print "Dog.name: "
puts Dog.name
print "Dog.superclass.name: "
puts Dog.superclass.name
print "Dog.ancestors: "
ancestors = Dog.ancestors.map { |c| c.name }
puts ancestors

puts ""
puts "=== Symbol#to_sym ==="
sym = :my_symbol
print ":my_symbol.to_sym == :my_symbol: "
puts sym.to_sym == :my_symbol

puts ""
puts "=== String#to_f ==="
s1 = "3.14"
print s1 + ".to_f: "
puts s1.to_f
s2 = "42.5"
print s2 + ".to_f: "
puts s2.to_f
s3 = "-1.5"
print s3 + ".to_f: "
puts s3.to_f

puts ""
puts "=== respond_to (already was aliased) ==="
print "5.respond_to(:to_s): "
puts 5.respond_to(:to_s)
print "[].respond_to(:map): "
arr = []
puts arr.respond_to(:map)
