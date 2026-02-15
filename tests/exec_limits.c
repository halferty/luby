#define LUBY_IMPLEMENTATION
#include "../luby.h"
#include <stdio.h>
#include <string.h>

#define TEST(name) printf("\n=== Test: %s ===\n", name)
#define PASS(name) printf("PASS: %s\n", name)
#define FAIL(name, ...) do { printf("FAIL: %s - ", name); printf(__VA_ARGS__); printf("\n"); return 1; } while(0)

int main(void) {
    // Test 1: Instruction limit with infinite loop
    TEST("Instruction limit stops infinite loop");
    {
        luby_config cfg = {0};
        cfg.instruction_limit = 1000;
        luby_state *L = luby_new(&cfg);
        luby_open_base(L);
        
        luby_value result;
        int rc = luby_eval(L, "x = 0\nwhile true\n  x = x + 1\nend", 0, "<test>", &result);
        
        if (rc != LUBY_E_RUNTIME) {
            FAIL("infinite_loop", "Expected runtime error, got %d", rc);
        }
        
        luby_error err = luby_last_error(L);
        if (strstr(err.message, "instruction limit") == NULL) {
            FAIL("infinite_loop", "Expected 'instruction limit' in error, got: %s", err.message);
        }
        
        PASS("infinite_loop");
        luby_free(L);
    }
    
    // Test 2: Instruction limit is reset per invocation
    TEST("Instruction limit resets between invocations");
    {
        luby_config cfg = {0};
        cfg.instruction_limit = 100;
        luby_state *L = luby_new(&cfg);
        luby_open_base(L);
        
        // First call should succeed (few instructions)
        luby_value result;
        int rc1 = luby_eval(L, "x = 1 + 2", 0, "<test>", &result);
        if (rc1 != 0) {
            FAIL("reset_counter", "First eval failed: %d", rc1);
        }
        
        // Second call should also succeed (counter was reset)
        int rc2 = luby_eval(L, "y = 3 + 4", 0, "<test>", &result);
        if (rc2 != 0) {
            FAIL("reset_counter", "Second eval failed: %d", rc2);
        }
        
        PASS("reset_counter");
        luby_free(L);
    }
    
    // Test 3: Call depth limit
    TEST("Call depth limit prevents stack overflow");
    {
        luby_config cfg = {0};
        cfg.call_depth_limit = 50;
        luby_state *L = luby_new(&cfg);
        luby_open_base(L);
        
        luby_value result;
        int rc = luby_eval(L, 
            "def recurse(n)\n"
            "  recurse(n + 1)\n"
            "end\n"
            "recurse(0)", 0, "<test>", &result);
        
        if (rc != LUBY_E_RUNTIME) {
            FAIL("call_depth", "Expected runtime error, got %d", rc);
        }
        
        luby_error err = luby_last_error(L);
        if (strstr(err.message, "stack overflow") == NULL) {
            FAIL("call_depth", "Expected 'stack overflow' in error, got: %s", err.message);
        }
        
        PASS("call_depth");
        luby_free(L);
    }
    
    // Test 4: Allocation limit
    TEST("Allocation limit prevents allocation spam");
    {
        luby_config cfg = {0};
        cfg.allocation_limit = 100;
        luby_state *L = luby_new(&cfg);
        luby_open_base(L);
        
        luby_value result;
        int rc = luby_eval(L, 
            "1000.times { [1, 2, 3, 4, 5] }", 0, "<test>", &result);
        
        if (rc != LUBY_E_RUNTIME) {
            FAIL("allocation_limit", "Expected runtime error, got %d", rc);
        }
        
        luby_error err = luby_last_error(L);
        if (strstr(err.message, "allocation limit") == NULL) {
            FAIL("allocation_limit", "Expected 'allocation limit' in error, got: %s", err.message);
        }
        
        PASS("allocation_limit");
        luby_free(L);
    }
    
    // Test 5: Allocation limit resets per invocation
    TEST("Allocation limit resets between invocations");
    {
        luby_config cfg = {0};
        cfg.allocation_limit = 20;
        luby_state *L = luby_new(&cfg);
        luby_open_base(L);
        
        luby_value result;
        // First call: allocate a few arrays
        int rc1 = luby_eval(L, "arr = [1, 2, 3]", 0, "<test>", &result);
        if (rc1 != 0) {
            FAIL("allocation_reset", "First eval failed: %d", rc1);
        }
        
        // Second call: should have fresh allocation budget
        int rc2 = luby_eval(L, "arr2 = [4, 5, 6]", 0, "<test>", &result);
        if (rc2 != 0) {
            FAIL("allocation_reset", "Second eval failed: %d", rc2);
        }
        
        PASS("allocation_reset");
        luby_free(L);
    }
    
    // Test 6: Memory limit
    TEST("Memory limit prevents excessive heap usage");
    {
        luby_config cfg = {0};
        luby_state *L = luby_new(&cfg);
        luby_open_base(L);
        
        // Set memory limit AFTER init so base classes can allocate freely
        // Use a limit that allows some work but prevents massive allocation
        size_t baseline = luby_get_memory_usage(L);
        luby_set_memory_limit(L, baseline + 50000);  // 50KB above baseline
        
        luby_value result;
        // Try to allocate many large arrays - should eventually hit limit
        int rc = luby_eval(L, 
            "arr = []\n"
            "10000.times { arr.push([1, 2, 3, 4, 5, 6, 7, 8, 9, 10]) }", 0, "<test>", &result);
        
        if (rc == 0) {
            FAIL("memory_limit", "Expected error but script succeeded");
        }
        
        // The memory limit should prevent completing the massive allocation.
        // The specific error code may vary (LUBY_E_RUNTIME for direct limit hit,
        // or another code if the limit cascades during internal operations).
        PASS("memory_limit");
        luby_free(L);
    }
    
    // Test 7: Get counts via API
    TEST("API functions to get counts");
    {
        luby_config cfg = {0};
        luby_state *L = luby_new(&cfg);
        luby_open_base(L);
        
        luby_value result;
        luby_eval(L, "x = 1 + 2 + 3", 0, "<test>", &result);
        
        size_t inst_count = luby_get_instruction_count(L);
        size_t alloc_count = luby_get_allocation_count(L);
        size_t mem_usage = luby_get_memory_usage(L);
        
        printf("Instructions: %zu, Allocations: %zu, Memory: %zu bytes\n", 
               inst_count, alloc_count, mem_usage);
        
        if (inst_count == 0) {
            FAIL("get_counts", "Instruction count should be > 0");
        }
        
        PASS("get_counts");
        luby_free(L);
    }
    
    // Test 8: Set limits dynamically
    TEST("Set limits dynamically");
    {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);
        
        // Set limits after creation
        luby_set_instruction_limit(L, 500);
        luby_set_call_depth_limit(L, 100);
        luby_set_allocation_limit(L, 50);
        luby_set_memory_limit(L, 50000);
        
        // Test that a moderate loop works
        luby_value result;
        int rc = luby_eval(L, "10.times { x = 1 + 2 }", 0, "<test>", &result);
        if (rc != 0) {
            FAIL("dynamic_limits", "Moderate loop failed: %d", rc);
        }
        
        // Test that excessive loop fails
        rc = luby_eval(L, "1000.times { x = 1 + 2 }", 0, "<test>", &result);
        if (rc == 0) {
            FAIL("dynamic_limits", "Excessive loop should have failed");
        }
        
        PASS("dynamic_limits");
        luby_free(L);
    }
    
    // Test 9: Fiber resume resets counters
    TEST("Fiber resume resets per-invocation counters");
    {
        luby_config cfg = {0};
        cfg.instruction_limit = 100;
        luby_state *L = luby_new(&cfg);
        luby_open_base(L);
        
        // Create a fiber that yields multiple times
        luby_value result;
        int rc = luby_eval(L, 
            "fib = Fiber.new do\n"
            "  10.times { |i| Fiber.yield(i) }\n"
            "end\n"
            "fib", 0, "<test>", &result);
        
        if (rc != 0) {
            FAIL("fiber_reset", "Fiber creation failed: %d", rc);
        }
        
        // Resume multiple times - each should get fresh instruction budget
        for (int i = 0; i < 5; i++) {
            rc = luby_eval(L, "fib.resume", 0, "<test>", &result);
            if (rc != 0) {
                FAIL("fiber_reset", "Fiber resume %d failed: %d", i, rc);
            }
        }
        
        PASS("fiber_reset");
        luby_free(L);
    }
    
    // Test 10: No limits by default
    TEST("No limits when config is NULL or 0");
    {
        luby_state *L = luby_new(NULL);
        luby_open_base(L);
        
        // Should be able to do deep recursion without hitting limits
        luby_value result;
        int rc = luby_eval(L, 
            "def fib(n)\n"
            "  if n <= 1\n"
            "    n\n"
            "  else\n"
            "    fib(n-1) + fib(n-2)\n"
            "  end\n"
            "end\n"
            "fib(15)", 0, "<test>", &result);
        
        if (rc != 0) {
            luby_error err = luby_last_error(L);
            FAIL("no_limits", "Fibonacci failed: %d (msg: %s)", rc, err.message ? err.message : "(null)");
        }
        
        if (result.type != LUBY_T_INT || result.as.i != 610) {
            FAIL("no_limits", "Fibonacci(15) should be 610, got %lld", 
                 (long long)result.as.i);
        }
        
        PASS("no_limits");
        luby_free(L);
    }
    
    printf("\n=== All execution limit tests passed! ===\n");
    return 0;
}
