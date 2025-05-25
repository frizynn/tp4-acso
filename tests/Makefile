CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE

# Integration test (root level)
TEST_INTEGRATION = test_integration

# Source files
INTEGRATION_SRC = test_integration.c

# Main targets
.PHONY: all ej1 ej2 integration clean help test-all clean-all

all: ej1 ej2 integration

# Build tests for each exercise
ej1:
	@echo "Building EJ1 (Ring) tests..."
	@$(MAKE) -C ej1

ej2:
	@echo "Building EJ2 (Shell) tests..."
	@$(MAKE) -C ej2

# Integration test
integration: $(TEST_INTEGRATION)

$(TEST_INTEGRATION): $(INTEGRATION_SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Test execution targets
.PHONY: test-ej1 test-ej2 test-integration

# Run all tests
test-all: all
	@echo "=========================================="
	@echo "      COMPREHENSIVE TEST SUITE"
	@echo "=========================================="
	@echo ""
	@echo "Running EJ1 (Ring Communication) tests..."
	@$(MAKE) -C ej1 test
	@echo ""
	@echo "Running EJ2 (Shell Implementation) tests..."
	@$(MAKE) -C ej2 test
	@echo ""
	@echo "Running Integration tests..."
	@$(MAKE) test-integration
	@echo ""
	@echo "=========================================="
	@echo "      ALL TESTS COMPLETED"
	@echo "=========================================="

# Run tests for specific exercises
test-ej1: ej1
	@echo "Running EJ1 (Ring Communication) tests..."
	@$(MAKE) -C ej1 test

test-ej2: ej2
	@echo "Running EJ2 (Shell Implementation) tests..."
	@$(MAKE) -C ej2 test

test-integration: integration
	@echo "Running Integration tests..."
	./$(TEST_INTEGRATION)

# Cleanup
clean:
	rm -f $(TEST_INTEGRATION) *.o core

clean-all: clean
	@$(MAKE) -C ej1 clean
	@$(MAKE) -C ej2 clean

# Help target
help:
	@echo "Available targets:"
	@echo "  all              - Build all tests (ej1, ej2, integration)"
	@echo "  ej1              - Build EJ1 (Ring Communication) tests"
	@echo "  ej2              - Build EJ2 (Shell Implementation) tests"
	@echo "  integration      - Build integration tests"
	@echo ""
	@echo "Test execution:"
	@echo "  test-all         - Run comprehensive test suite"
	@echo "  test-ej1         - Run EJ1 tests only"
	@echo "  test-ej2         - Run EJ2 tests only"
	@echo "  test-integration - Run integration tests only"
	@echo ""
	@echo "Cleanup:"
	@echo "  clean            - Remove built files in root directory"
	@echo "  clean-all        - Remove all built files (including subdirectories)"
	@echo "  help             - Show this help"
