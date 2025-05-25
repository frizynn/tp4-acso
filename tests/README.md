# Test Documentation

This document describes the comprehensive Test-Driven Development (TDD) implementation for the Operating Systems course TP-4.

## Project Structure

```
tests/
├── Makefile           # Build system for tests
├── test_ring.c        # Tests for ring communication (Exercise 1)
├── test_shell.c       # Tests for shell implementation (Exercise 2)
└── README.md          # This file

src/
├── ej1/
│   ├── ring.c         # Ring communication implementation
│   └── Makefile       # Build system for ring
└── ej2/
    ├── shell.c        # Shell implementation
    └── Makefile       # Build system for shell
```

## TDD Approach

The TDD methodology was followed for both exercises:

1. **Red Phase**: Write failing tests first
2. **Green Phase**: Implement minimal code to make tests pass
3. **Refactor Phase**: Improve code while keeping tests green

## Exercise 1: Ring Communication Tests

### Test Coverage

1. **Argument Validation**
   - `ring_accepts_correct_arguments`: Validates usage message
   - `ring_invalid_arguments`: Tests error handling for wrong argument count

2. **Basic Functionality**
   - `ring_basic_functionality_3_processes`: Tests 3-process ring with value 5
   - `ring_different_start_process`: Tests different starting process
   - `ring_with_zero_value`: Tests with initial value 0

3. **Edge Cases**
   - `ring_minimum_size`: Tests minimum ring size (2 processes)
   - `ring_large_number_processes`: Tests with 10 processes

4. **Expected Behavior**
   - Each process increments the value by 1
   - Final result = initial_value + number_of_processes
   - Process creation and communication via pipes

### Ring Implementation Features

- **Process Creation**: Creates n child processes using fork()
- **Inter-Process Communication**: Uses pipes for message passing
- **Sequential Processing**: Each process increments and passes the value
- **Result Collection**: Parent waits for all children and prints result

## Exercise 2: Shell Implementation Tests

### Test Coverage

1. **Basic Shell Functions**
   - `shell_displays_prompt`: Verifies shell starts and shows prompt
   - `shell_handles_empty_input`: Tests empty command handling
   - `shell_handles_exit`: Tests exit command functionality

2. **Command Parsing**
   - `shell_parses_single_command`: Tests single command parsing
   - `shell_parses_pipe_commands`: Tests two commands with pipe
   - `shell_parses_multiple_pipes`: Tests three commands with pipes

3. **Command Execution**
   - `shell_executes_simple_command`: Tests single command execution
   - `shell_executes_pipe_command`: Tests piped command execution

### Shell Implementation Features

- **Interactive Prompt**: Displays "Shell> " prompt
- **Command Parsing**: Tokenizes input by pipe character (|)
- **Pipe Support**: Handles multiple commands connected by pipes
- **Process Management**: Creates child processes for command execution
- **Built-in Commands**: Supports exit command
- **Error Handling**: Graceful handling of invalid commands

## Running Tests

### Individual Tests
```bash
make test-ring    # Run only ring tests
make test-shell   # Run only shell tests
```

### All Tests
```bash
make test         # Run all tests
```

### Clean Build
```bash
make clean        # Remove test executables
```

## Test Framework

The tests use a simple custom framework with:

- **TEST(name)**: Macro to define test functions
- **RUN_TEST(name)**: Macro to execute tests with reporting
- **assert()**: Standard C assertions for validation
- **capture_output()**: Helper function to capture program output

## Expected Results

All tests should pass with green checkmarks (✓). The test output shows:
- Test name being executed
- Success confirmation for each test
- Final summary of all tests passed

## Test Philosophy

These tests ensure:

1. **Correctness**: Programs work as specified
2. **Robustness**: Proper error handling for edge cases
3. **Reliability**: Consistent behavior across different inputs
4. **Maintainability**: Easy to add new tests as features grow

## Future Enhancements

Potential additional tests could include:
- Performance testing with large process counts
- Memory leak detection
- Signal handling tests
- More complex shell command combinations
- Input/output redirection tests
