#!/bin/bash

# Test runner for Luby
# Compiles and runs all tests

set -e  # Exit on first error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PASSED=0
FAILED=0
TOTAL=0

echo "=================================="
echo "  Luby Test Suite"
echo "=================================="
echo ""

run_test() {
    local test_name=$1
    local test_file="tests/${test_name}.c"
    local test_bin="tests/${test_name}"
    
    if [ ! -f "$test_file" ]; then
        echo -e "${YELLOW}SKIP${NC} $test_name (file not found)"
        return
    fi
    
    TOTAL=$((TOTAL + 1))
    
    echo -n "Building $test_name... "
    if gcc -o "$test_bin" "$test_file" -I. -std=c99 -lm 2>/dev/null; then
        echo "done"
        echo -n "Running $test_name... "
        
        if ./"$test_bin" > /tmp/luby_test_${test_name}.log 2>&1; then
            echo -e "${GREEN}PASS${NC}"
            PASSED=$((PASSED + 1))
        else
            echo -e "${RED}FAIL${NC}"
            FAILED=$((FAILED + 1))
            echo "  Output:"
            cat /tmp/luby_test_${test_name}.log | sed 's/^/    /'
        fi
        rm -f /tmp/luby_test_${test_name}.log
    else
        echo -e "${RED}FAIL${NC} (compilation failed)"
        FAILED=$((FAILED + 1))
    fi
    echo ""
}

# Run all tests
run_test "basic"
run_test "features"
run_test "missing"
run_test "more"
run_test "vfs"
run_test "reflection"
run_test "file_line"
run_test "visibility"
run_test "numeric"
run_test "game_math"
run_test "string_array"
run_test "euler"
run_test "rosetta"

# Summary
echo "=================================="
echo "  Test Results"
echo "=================================="
echo -e "Total:  $TOTAL"
echo -e "Passed: ${GREEN}$PASSED${NC}"
echo -e "Failed: ${RED}$FAILED${NC}"
echo "=================================="

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
fi
