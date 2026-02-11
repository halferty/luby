# Makefile for Luby tests

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -I.
LDFLAGS = -lm

# Test sources and binaries
TEST_SOURCES = $(wildcard tests/*.c)
TEST_BINS = $(TEST_SOURCES:.c=)

.PHONY: all test clean help

all: test

# Build all tests
tests: $(TEST_BINS)

# Pattern rule for building test executables
tests/%: tests/%.c luby.h
	@echo "Building $@..."
	@$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Run all tests
test: tests
	@./run_tests.sh

# Clean build artifacts
clean:
	@echo "Cleaning..."
	@rm -f $(TEST_BINS)
	@rm -rf tests/*.dSYM
	@rm -f test_basic test_features test_missing
	@echo "Done."

# Help target
help:
	@echo "Luby Test Suite Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make test    - Build and run all tests"
	@echo "  make tests   - Build all test executables"
	@echo "  make clean   - Remove build artifacts"
	@echo "  make help    - Show this help message"
	@echo ""
	@echo "Individual tests:"
	@echo "  tests/basic"
	@echo "  tests/features"
	@echo "  tests/missing"
	@echo "  tests/more"
	@echo "  tests/vfs"
	@echo "  tests/reflection"
	@echo "  tests/file_line"
	@echo "  tests/visibility"
	@echo "  tests/numeric"
	@echo "  tests/game_math"
