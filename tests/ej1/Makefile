# Makefile for Ring Communication Tests (Exercise 1)
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE
RING_BASIC_TESTS = test_ring test_ring_advanced test_ring_advanced_strict test_ring_extreme
EXECUTABLES = $(RING_BASIC_TESTS)

.PHONY: all clean test

all: $(EXECUTABLES)

# Basic ring tests
test_ring: test_ring.c
	$(CC) $(CFLAGS) -o $@ $<

# Advanced ring tests (refactored from test_advanced)
test_ring_advanced: test_ring_advanced.c
	$(CC) $(CFLAGS) -o $@ $<

# Strict ring tests
test_ring_advanced_strict: test_ring_advanced_strict.c
	$(CC) $(CFLAGS) -o $@ $<

# Extreme ring tests
test_ring_extreme: test_ring_extreme.c
	$(CC) $(CFLAGS) -o $@ $<

# Run all ring tests
test: all
	@echo "=================================================="
	@echo "Running Ring Communication Tests (Exercise 1)"
	@echo "=================================================="
	@echo
	@for test in $(EXECUTABLES); do \
		if [ -f $$test ]; then \
			echo "Running $$test..."; \
			./$$test; \
			echo; \
		fi; \
	done
	@echo "✅ All ring tests completed!"

clean:
	rm -f $(EXECUTABLES)
	rm -f *.o core
