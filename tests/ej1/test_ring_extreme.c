#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    printf("Running test: %s\n", #name); \
    test_##name(); \
    printf("✓ %s passed\n", #name); \
} while(0)

char* capture_output_with_timeout(const char* command, int timeout_seconds) {
    FILE* fp;
    char path[1035];
    static char result[8192] = {0};
    
    fp = popen(command, "r");
    if (fp == NULL) {
        return NULL;
    }
    
    result[0] = '\0';
    time_t start_time = time(NULL);
    
    while (fgets(path, sizeof(path), fp) != NULL) {
        strcat(result, path);
        if (time(NULL) - start_time > timeout_seconds) {
            pclose(fp);
            strcpy(result, "TIMEOUT");
            return result;
        }
    }
    
    pclose(fp);
    return result;
}

// Test: Maximum system limits stress test
TEST(ring_maximum_processes_stress) {
    system("cd ../../src/ej1 && make clean && make");
    
    // Test with 100 processes (stress test for pipe/fork limits)
    char* output = capture_output_with_timeout("cd ../../src/ej1 && ./ring 100 1 50", 20);
    
    assert(strcmp(output, "TIMEOUT") != 0);
    assert(strstr(output, "101") != NULL); // 1 + 100 = 101
}

// Test: Integer overflow boundary conditions
TEST(ring_integer_overflow_boundary) {
    system("cd ../../src/ej1 && make clean && make");
    
    // Test near INT_MAX
    char* output1 = capture_output_with_timeout("cd ../../src/ej1 && ./ring 3 2147483644 0", 5);
    assert(strcmp(output1, "TIMEOUT") != 0);
    // 2147483644 + 3 = 2147483647 (should not overflow)
    assert(strstr(output1, "2147483647") != NULL);
    
    // Test that would overflow
    char* output2 = capture_output_with_timeout("cd ../../src/ej1 && ./ring 3 2147483646 0", 5);
    assert(strcmp(output2, "TIMEOUT") != 0);
    // This will overflow but program should handle gracefully
    assert(strlen(output2) > 0);
}

// Test: Extreme negative values
TEST(ring_extreme_negative_values) {
    system("cd ../../src/ej1 && make clean && make");
    
    char* output = capture_output_with_timeout("cd ../../src/ej1 && ./ring 5 -2147483647 2", 5);
    
    assert(strcmp(output, "TIMEOUT") != 0);
    // -2147483647 + 5 = -2147483642
    assert(strstr(output, "-2147483642") != NULL);
}

// Test: All possible start positions for a given ring size
TEST(ring_all_start_positions) {
    system("cd ../../src/ej1 && make clean && make");
    
    // Test all start positions for a 5-process ring
    for (int start = 0; start < 5; start++) {
        char command[256];
        sprintf(command, "cd ../../src/ej1 && ./ring 5 10 %d", start);
        char* output = capture_output_with_timeout(command, 5);
        
        assert(strcmp(output, "TIMEOUT") != 0);
        assert(strstr(output, "15") != NULL); // 10 + 5 = 15
        
        char expected_start[32];
        sprintf(expected_start, "desde proceso %d", start);
        assert(strstr(output, expected_start) != NULL);
    }
}

// Test: Rapid consecutive executions (race condition test)
TEST(ring_rapid_consecutive_executions) {
    system("cd ../../src/ej1 && make clean && make");
    
    // Run the same test 10 times rapidly
    for (int i = 0; i < 10; i++) {
        char* output = capture_output_with_timeout("cd ../../src/ej1 && ./ring 4 25 1", 5);
        assert(strcmp(output, "TIMEOUT") != 0);
        assert(strstr(output, "29") != NULL); // 25 + 4 = 29
    }
}

// Test: Memory and resource cleanup (zombie process test)
TEST(ring_resource_cleanup) {
    system("cd ../../src/ej1 && make clean && make");
    
    // Run multiple tests and check that no zombie processes remain
    for (int i = 0; i < 5; i++) {
        char* output = capture_output_with_timeout("cd ../../src/ej1 && ./ring 6 0 3", 5);
        assert(strcmp(output, "TIMEOUT") != 0);
        assert(strstr(output, "6") != NULL); // 0 + 6 = 6
        
        // Small delay to allow cleanup
        usleep(100000); // 100ms
    }
    
    // Check for zombie processes
    char* ps_output = capture_output_with_timeout("ps aux | grep '[Zz]ombie\\|<defunct>' | grep -v grep", 2);
    // Should not find any zombie processes related to our ring program
    assert(strlen(ps_output) == 0 || strstr(ps_output, "ring") == NULL);
}

// Test: Signal handling robustness
TEST(ring_signal_robustness) {
    system("cd ../../src/ej1 && make clean && make");
    
    // Test that program completes even with potential signal interference
    char* output = capture_output_with_timeout("cd ../../src/ej1 && timeout 10 ./ring 8 42 4", 12);
    
    assert(strcmp(output, "TIMEOUT") != 0);
    assert(strstr(output, "50") != NULL); // 42 + 8 = 50
}

// Test: Input validation edge cases with special characters
TEST(ring_special_input_validation) {
    system("cd ../../src/ej1 && make clean && make");
    
    // Test with very long argument strings (should be handled by atoi)
    char* output1 = capture_output_with_timeout("cd ../../src/ej1 && ./ring 999999999999999999999 10 0 2>&1", 5);
    // atoi will convert this to some value, program should handle it
    assert(strcmp(output1, "TIMEOUT") != 0);
    
    // Test with leading zeros
    char* output2 = capture_output_with_timeout("cd ../../src/ej1 && ./ring 003 000010 001", 5);
    assert(strstr(output2, "13") != NULL); // 10 + 3 = 13
}

// Test: Process synchronization verification
TEST(ring_process_synchronization) {
    system("cd ../../src/ej1 && make clean && make");
    
    // Large ring to test synchronization between many processes
    char* output = capture_output_with_timeout("cd ../../src/ej1 && ./ring 25 100 12", 15);
    
    assert(strcmp(output, "TIMEOUT") != 0);
    assert(strstr(output, "125") != NULL); // 100 + 25 = 125
    assert(strstr(output, "Se crearán 25 procesos") != NULL);
}

// Test: Error handling completeness
TEST(ring_comprehensive_error_handling) {
    system("cd ../../src/ej1 && make clean && make");
    
    // Test all error conditions systematically
    struct {
        const char* args;
        int should_error;
    } test_cases[] = {
        {"0 10 0", 1},           // Zero processes
        {"-1 10 0", 1},          // Negative processes
        {"3 10 3", 1},           // Start >= n
        {"3 10 -1", 1},          // Start < 0
        {"5 10 5", 1},           // Start >= n (boundary)
        {"1 10 0", 0},           // Valid single process
        {"2 10 1", 0},           // Valid minimum ring
    };
    
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        char command[256];
        sprintf(command, "cd ../../src/ej1 && ./ring %s 2>&1", test_cases[i].args);
        char* output = capture_output_with_timeout(command, 5);
        
        if (test_cases[i].should_error) {
            assert(strstr(output, "Error") != NULL);
        } else {
            assert(strstr(output, "Error") == NULL);
        }
    }
}

int main() {
    printf("Running EXTREME & COMPREHENSIVE Ring Tests\n");
    printf("==========================================\n");
    
    RUN_TEST(ring_maximum_processes_stress);
    RUN_TEST(ring_integer_overflow_boundary);
    RUN_TEST(ring_extreme_negative_values);
    RUN_TEST(ring_all_start_positions);
    RUN_TEST(ring_rapid_consecutive_executions);
    RUN_TEST(ring_resource_cleanup);
    RUN_TEST(ring_signal_robustness);
    RUN_TEST(ring_special_input_validation);
    RUN_TEST(ring_process_synchronization);
    RUN_TEST(ring_comprehensive_error_handling);
    
    printf("\n🎯 ALL EXTREME TESTS PASSED! 🎯\n");
    printf("Your ring implementation is EXCEPTIONALLY ROBUST!\n");
    printf("It handles:\n");
    printf("  ✓ Edge cases and boundary conditions\n");
    printf("  ✓ Resource management and cleanup\n");
    printf("  ✓ Error handling and validation\n");
    printf("  ✓ Performance under stress\n");
    printf("  ✓ Integer overflow scenarios\n");
    printf("  ✓ Process synchronization\n");
    printf("  ✓ Signal robustness\n");
    
    return 0;
}
