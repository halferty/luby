# Demonstration of numeric predicates

puts "=== Zero Check ==="
[0, 5, -3, 0.0].each do |n|
  puts "#{n}.zero? = #{n.zero?}"
end

puts ""
puts "=== Positive/Negative Checks ==="
[-5, 0, 10, -3.5, 2.7].each do |n|
  pos = n.positive?
  neg = n.negative?
  puts "#{n}: positive? = #{pos}, negative? = #{neg}"
end

puts ""
puts "=== Even/Odd Checks ==="
[1, 2, 3, 4, 5, -2, -3, 0].each do |n|
  puts "#{n}: even? = #{n.even?}, odd? = #{n.odd?}"
end

puts ""
puts "=== Abs, Ceil, Floor, Round ==="
values = [-5, 3.2, -3.8, 3.5, 3.4, -2.5]
values.each do |n|
  puts "#{n}: abs = #{abs(n)}, ceil = #{ceil(n)}, floor = #{floor(n)}, round = #{round(n)}"
end

puts ""
puts "=== Practical Examples ==="

# Check if a number is within range
def in_range?(n, min, max)
  !n.negative? && n >= min && n <= max
end

puts "5 in range 0-10? #{in_range?(5, 0, 10)}"
puts "15 in range 0-10? #{in_range?(15, 0, 10)}"

# Count evens and odds
numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
even_count = 0
odd_count = 0

numbers.each do |n|
  if n.even?
    even_count = even_count + 1
  else
    odd_count = odd_count + 1
  end
end

puts ""
puts "From #{numbers.length} numbers:"
puts "  Evens: #{even_count}"
puts "  Odds: #{odd_count}"

# Temperature check
def describe_temp(temp)
  if temp.zero?
    "freezing point"
  elsif temp.positive?
    "above freezing"
  else
    "below freezing"
  end
end

puts ""
[-5, 0, 10].each do |temp|
  puts "#{temp}°C is #{describe_temp(temp)}"
end

puts ""
puts "Done!"
