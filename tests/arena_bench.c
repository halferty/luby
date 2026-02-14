#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>
#include <time.h>

// Code that exercises the parser/compiler with various constructs
const char *TEST_CODE = 
    "class Person\n"
    "  attr_reader :name, :age\n"
    "  def initialize(name, age)\n"
    "    @name = name\n"
    "    @age = age\n"
    "  end\n"
    "  def older_than(other)\n"
    "    @age > other.age\n"
    "  end\n"
    "end\n"
    "\n"
    "people = [Person.new('Alice', 30), Person.new('Bob', 25), Person.new('Charlie', 35)]\n"
    "\n"
    "total_age = 0\n"
    "people.each { |p| total_age = total_age + p.age }\n"
    "\n"
    "oldest = people[0]\n"
    "people.each do |p|\n"
    "  if p.older_than(oldest)\n"
    "    oldest = p\n"
    "  end\n"
    "end\n"
    "\n"
    "total_age\n";  // Should be 90

#define ITERATIONS 10000

int main() {
    printf("=== Arena Allocation Benchmark ===\n\n");
    printf("Code complexity: ~35 lines with classes, methods, blocks, arrays\n");
    printf("Iterations: %d\n\n", ITERATIONS);
    
    // Warm up and verify correctness
    luby_state *L = luby_new(NULL);
    luby_open_base(L);
    luby_value result;
    int rc = luby_eval(L, TEST_CODE, 0, "<bench>", &result);
    if (rc != 0 || result.type != LUBY_T_INT || result.as.i != 90) {
        char buf[256];
        luby_format_error(L, buf, sizeof(buf));
        printf("ERROR: Test code failed (rc=%d, expected 90, got %lld)\n", 
               rc, result.type == LUBY_T_INT ? result.as.i : -1);
        printf("Error: %s\n", buf);
        luby_free(L);
        return 1;
    }
    printf("Correctness verified: result = %lld (expected 90)\n\n", result.as.i);
    luby_free(L);
    
    // Benchmark
    clock_t start = clock();
    
    for (int i = 0; i < ITERATIONS; i++) {
        L = luby_new(NULL);
        luby_open_base(L);
        luby_eval(L, TEST_CODE, 0, "<bench>", &result);
        luby_free(L);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Time for %d iterations: %.3f seconds\n", ITERATIONS, elapsed);
    printf("Average per iteration: %.3f ms\n", (elapsed / ITERATIONS) * 1000);
    printf("Iterations per second: %.0f\n", ITERATIONS / elapsed);
    
    return 0;
}
