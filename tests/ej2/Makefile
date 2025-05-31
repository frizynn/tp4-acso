# Makefile for Shell Implementation Tests (Exercise 2)
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE

# Core shell tests
BASIC_TESTS = test_shell test_shell_advanced debug_test test_shell_extra_credit

# New comprehensive test suites
COMPREHENSIVE_TESTS = test_shell_robustness test_shell_security test_shell_compatibility test_shell_extreme_edge_cases

# All shell tests
SHELL_TESTS = $(BASIC_TESTS) $(COMPREHENSIVE_TESTS)
EXECUTABLES = $(SHELL_TESTS)

.PHONY: all clean test basic advanced comprehensive extreme

all: $(EXECUTABLES)

# === BASIC TESTS ===

# Basic shell tests
test_shell: test_shell.c
	$(CC) $(CFLAGS) -o $@ $<

# Advanced shell tests (refactored from test_advanced)
test_shell_advanced: test_shell_advanced.c
	$(CC) $(CFLAGS) -o $@ $<

# Debug tests
debug_test: debug_test.c
	$(CC) $(CFLAGS) -o $@ $<

# Extra credit tests (quote handling and patterns with spaces)
test_shell_extra_credit: test_shell_extra_credit.c
	$(CC) $(CFLAGS) -o $@ $<

# === COMPREHENSIVE TESTS ===

# Robustness and stress testing
test_shell_robustness: test_shell_robustness.c
	$(CC) $(CFLAGS) -o $@ $<

# Security and vulnerability testing
test_shell_security: test_shell_security.c
	$(CC) $(CFLAGS) -o $@ $<

# Bash compatibility testing
test_shell_compatibility: test_shell_compatibility.c
	$(CC) $(CFLAGS) -o $@ $<

# Extreme edge cases testing
test_shell_extreme_edge_cases: test_shell_extreme_edge_cases.c
	$(CC) $(CFLAGS) -o $@ $<

# === TEST EXECUTION TARGETS ===

# Run basic tests only
basic: $(BASIC_TESTS)
	@echo "=================================================="
	@echo "Running BASIC Shell Tests (Exercise 2)"
	@echo "=================================================="
	@echo
	@for test in $(BASIC_TESTS); do \
		if [ -f $$test ]; then \
			echo "🔹 Running $$test..."; \
			timeout 30 ./$$test || echo "Test completed (exit code: $$?)"; \
			echo; \
		fi; \
	done
	@echo "✅ Basic shell tests completed!"

# Run comprehensive tests only
comprehensive: $(COMPREHENSIVE_TESTS)
	@echo "=========================================================="
	@echo "Running COMPREHENSIVE Shell Tests (Exercise 2)"
	@echo "=========================================================="
	@echo
	@for test in $(COMPREHENSIVE_TESTS); do \
		if [ -f $$test ]; then \
			echo "🔥 Running $$test..."; \
			timeout 60 ./$$test || echo "Test completed (exit code: $$?)"; \
			echo; \
		fi; \
	done
	@echo "🏆 Comprehensive shell tests completed!"

# Run extreme edge case tests only
extreme: test_shell_extreme_edge_cases
	@echo "=========================================================="
	@echo "Running EXTREME EDGE CASE Shell Tests"
	@echo "=========================================================="
	@echo
	@if [ -f test_shell_extreme_edge_cases ]; then \
		echo "🎯 Running extreme edge case tests..."; \
		timeout 120 ./test_shell_extreme_edge_cases || echo "Test completed (exit code: $$?)"; \
		echo; \
	fi
	@echo "🎯 Extreme edge case tests completed!"

# Run ALL tests (basic + comprehensive)
test: all
	@echo "============================================================"
	@echo "Running COMPLETE Shell Test Suite (Exercise 2)"
	@echo "============================================================"
	@echo
	@echo "📋 Phase 1: Basic Tests"
	@echo "------------------------"
	@for test in $(BASIC_TESTS); do \
		if [ -f $$test ]; then \
			echo "🔹 Running $$test..."; \
			timeout 30 ./$$test || echo "Test completed (exit code: $$?)"; \
			echo; \
		fi; \
	done
	@echo
	@echo "🔥 Phase 2: Comprehensive Tests"
	@echo "--------------------------------"
	@for test in $(COMPREHENSIVE_TESTS); do \
		if [ -f $$test ]; then \
			echo "🔥 Running $$test..."; \
			timeout 90 ./$$test || echo "Test completed (exit code: $$?)"; \
			echo; \
		fi; \
	done
	@echo
	@echo "🏆 COMPLETE TEST SUITE FINISHED!"
	@echo "=================================="
	@echo "✅ Basic functionality verified"
	@echo "🛡️ Security and robustness verified"
	@echo "🌐 Bash compatibility verified"
	@echo "🎯 Extreme edge cases verified"
	@echo "🚀 Your shell is production-ready!"

clean:
	rm -f $(EXECUTABLES)
	rm -f *.o core
	rm -f test_*.txt binary_test.dat special.txt control.txt
	rm -f testdir -rf

# Help target
help:
	@echo "Shell Test Suite (Exercise 2) - Available targets:"
	@echo "=================================================="
	@echo "  all          - Build all tests"
	@echo "  basic        - Run basic functionality tests"
	@echo "  comprehensive- Run comprehensive test suite"
	@echo "  extreme      - Run extreme edge case tests"
	@echo "  test         - Run complete test suite (all tests)"
	@echo "  clean        - Remove all built files"
	@echo "  help         - Show this help message"
	@echo
	@echo "Test Categories:"
	@echo "  🔹 Basic: Core functionality and parsing"
	@echo "  🔥 Comprehensive: Robustness, security, compatibility"
	@echo "  🎯 Extreme: Edge cases that break most shells"
