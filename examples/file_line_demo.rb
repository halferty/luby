# Demonstration of __FILE__ and __LINE__ special variables

puts "Current file: #{__FILE__}"
puts "Current line: #{__LINE__}"

def show_location
  puts "Inside show_location at #{__FILE__}:#{__LINE__}"
end

show_location

class Logger
  def log(message)
    puts "[#{__FILE__}:#{__LINE__}] #{message}"
  end
end

logger = Logger.new
logger.log("This is a log message")

# Demonstrating line tracking
a = __LINE__
b = __LINE__
c = __LINE__

puts "Lines: a=#{a}, b=#{b}, c=#{c}"
