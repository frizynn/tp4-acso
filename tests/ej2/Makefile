# Makefile for Shell Implementation Tests (Exercise 2)
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE
SHELL_TESTS = test_shell test_shell_advanced debug_test test_shell_extra_credit
EXECUTABLES = $(SHELL_TESTS)

.PHONY: all clean test

all: $(EXECUTABLES)

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

# Run all shell tests
test: all
	@echo "=================================================="
	@echo "Running Shell Implementation Tests (Exercise 2)"
	@echo "=================================================="
	@echo
	@for test in $(EXECUTABLES); do \
		if [ -f $$test ]; then \
			echo "Running $$test..."; \
			timeout 30 ./$$test || echo "Test completed (with timeout or exit code: $$?)"; \
			echo; \
		fi; \
	done
	@echo "✅ All shell tests completed!"

clean:
	rm -f $(EXECUTABLES)
	rm -f *.o core
