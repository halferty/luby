# Demonstration of visibility modifiers and alias

puts "=== Visibility Modifiers Demo ==="

class BankAccount
  def initialize(balance)
    @balance = balance
  end
  
  # Public method (default)
  def deposit(amount)
    @balance = @balance + amount
    puts "Deposited #{amount}, new balance: #{@balance}"
  end
  
  # Private methods - can only be called within the class
  private
  
  def validate_transaction(amount)
    amount > 0
  end
  
  def log_transaction(type, amount)
    puts "[INTERNAL LOG] #{type}: #{amount}"
  end
  
  # Back to public
  public
  
  def withdraw(amount)
    if validate_transaction(amount)
      log_transaction("withdrawal", amount)
      @balance = @balance - amount
      puts "Withdrew #{amount}, new balance: #{@balance}"
    end
  end
  
  # Protected method
  protected
  
  def compare_balance(other)
    @balance - other.get_balance_internal
  end
  
  def get_balance_internal
    @balance
  end
end

account = BankAccount.new(100)
account.deposit(50)
account.withdraw(30)

puts ""
puts "=== Alias Demo ==="

class Calculator
  def add(a, b)
    a + b
  end
  
  def multiply(a, b)
    a * b
  end
  
  # Create aliases for methods
  alias plus add
  alias times multiply
  alias sum add
end

calc = Calculator.new
puts "add(2, 3) = #{calc.add(2, 3)}"
puts "plus(2, 3) = #{calc.plus(2, 3)}"
puts "sum(2, 3) = #{calc.sum(2, 3)}"
puts "multiply(4, 5) = #{calc.multiply(4, 5)}"
puts "times(4, 5) = #{calc.times(4, 5)}"

puts ""
puts "=== Changing Visibility After Definition ==="

class Example
  def foo
    "foo is public by default"
  end
  
  def bar
    "bar is also public"
  end
  
  # Make foo private
  private :foo
  
  def test
    foo  # Can call private method from within class
  end
end

ex = Example.new
puts ex.bar
puts ex.test

puts ""
puts "Done!"
