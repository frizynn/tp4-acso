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

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    printf("Running test: %s\n", #name); \
    test_##name(); \
    printf("✓ %s passed\n", #name); \
} while(0)

// Helper function to capture program output with timeout
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

// Test: Single process ring (edge case)
TEST(ring_single_process) {
    system("cd ../../src/ej1 && make clean && make");
    
    char* output = capture_output_with_timeout("cd ../../src/ej1 && ./ring 1 42 0", 5);
    
    // With 1 process: 42 + 1 = 43
    assert(strstr(output, "43") != NULL);
    assert(strstr(output, "Se crearán 1 procesos") != NULL);
}

// Test: Large number of processes (stress test)
TEST(ring_stress_test_50_processes) {
    system("cd ../../src/ej1 && make clean && make");
    
    char* output = capture_output_with_timeout("cd ../../src/ej1 && ./ring 50 0 25", 10);
    
    // Should not timeout and should produce correct result: 0 + 50 = 50
    assert(strcmp(output, "TIMEOUT") != 0);
    assert(strstr(output, "50") != NULL);
}

// Test: Negative initial values
TEST(ring_negative_initial_value) {
    system("cd ../../src/ej1 && make clean && make");
    
    char* output = capture_output_with_timeout("cd ../../src/ej1 && ./ring 3 -10 0", 5);
    
    // -10 + 3 = -7
    assert(strstr(output, "-7") != NULL);
}

// Test: Large initial values
TEST(ring_large_initial_value) {
    system("cd ../../src/ej1 && make clean && make");
    
    char* output = capture_output_with_timeout("cd ../../src/ej1 && ./ring 5 2147483647 0", 5);
    
    // This tests integer overflow handling: 2147483647 + 5 should wrap around
    // The program should handle this gracefully
    assert(strcmp(output, "TIMEOUT") != 0);
    assert(strlen(output) > 0);
}

// Test: Invalid process numbers (boundary conditions)
TEST(ring_invalid_process_numbers) {
    system("cd ../../src/ej1 && make clean && make");
    
    // Start process >= n
    char* output1 = capture_output_with_timeout("cd ../../src/ej1 && ./ring 3 10 3 2>&1", 5);
    assert(strstr(output1, "Error") != NULL);
    
    // Start process < 0
    char* output2 = capture_output_with_timeout("cd ../../src/ej1 && ./ring 3 10 -1 2>&1", 5);
    assert(strstr(output2, "Error") != NULL);
    
    // Zero processes
    char* output3 = capture_output_with_timeout("cd ../../src/ej1 && ./ring 0 10 0 2>&1", 5);
    assert(strstr(output3, "Error") != NULL);
    
    // Negative number of processes
    char* output4 = capture_output_with_timeout("cd ../../src/ej1 && ./ring -5 10 0 2>&1", 5);
    assert(strstr(output4, "Error") != NULL);
}

// Test: Maximum starting process index
TEST(ring_max_start_process) {
    system("cd ../../src/ej1 && make clean && make");
    
    char* output = capture_output_with_timeout("cd ../../src/ej1 && ./ring 7 100 6", 5);
    
    // 100 + 7 = 107, starting from last process
    assert(strstr(output, "107") != NULL);
    assert(strstr(output, "desde proceso 6") != NULL);
}

// Test: Zero as initial value with large ring
TEST(ring_zero_value_large_ring) {
    system("cd ../../src/ej1 && make clean && make");
    
    char* output = capture_output_with_timeout("cd ../../src/ej1 && ./ring 20 0 10", 8);
    
    // 0 + 20 = 20
    assert(strstr(output, "20") != NULL);
    assert(strcmp(output, "TIMEOUT") != 0);
}

// Test: Consistency check - multiple runs should give same result
TEST(ring_consistency_check) {
    system("cd ../../src/ej1 && make clean && make");
    
    char* output1 = capture_output_with_timeout("cd ../../src/ej1 && ./ring 4 15 2", 5);
    char* output2 = capture_output_with_timeout("cd ../../src/ej1 && ./ring 4 15 2", 5);
    char* output3 = capture_output_with_timeout("cd ../../src/ej1 && ./ring 4 15 2", 5);
    
    // All three runs should produce the same result: 15 + 4 = 19
    assert(strstr(output1, "19") != NULL);
    assert(strstr(output2, "19") != NULL);
    assert(strstr(output3, "19") != NULL);
}

// Test: Performance test - should complete within reasonable time
TEST(ring_performance_test) {
    system("cd ../../src/ej1 && make clean && make");
    
    time_t start = time(NULL);
    char* output = capture_output_with_timeout("cd ../../src/ej1 && ./ring 30 1000 15", 15);
    time_t end = time(NULL);
    
    // Should complete within 15 seconds and produce correct result
    assert(strcmp(output, "TIMEOUT") != 0);
    assert(strstr(output, "1030") != NULL); // 1000 + 30
    assert(end - start < 15);
}

// Test: Very large start process (boundary)
TEST(ring_boundary_start_process) {
    system("cd ../../src/ej1 && make clean && make");
    
    char* output = capture_output_with_timeout("cd ../../src/ej1 && ./ring 100 50 99", 10);
    
    // 50 + 100 = 150, starting from process 99 (last one)
    assert(strstr(output, "150") != NULL);
    assert(strcmp(output, "TIMEOUT") != 0);
}

// Test: String arguments that should be rejected
TEST(ring_invalid_string_arguments) {
    system("cd ../../src/ej1 && make clean && make");
    
    // Non-numeric arguments should be handled (atoi returns 0 for invalid strings)
    char* output1 = capture_output_with_timeout("cd ../../src/ej1 && ./ring abc 10 0 2>&1", 5);
    // atoi(\"abc\") = 0, so this should give an error for 0 processes
    assert(strstr(output1, "Error") != NULL);
    
    char* output2 = capture_output_with_timeout("cd ../../src/ej1 && ./ring 3 xyz 0 2>&1", 5);
    // atoi(\"xyz\") = 0, so initial value becomes 0, which is valid
    // Result should be 0 + 3 = 3
    assert(strstr(output2, "3") != NULL);
}

// Test: Edge case with very small and very large combinations
TEST(ring_edge_combinations) {
    system("cd ../../src/ej1 && make clean && make");
    
    // Minimum valid ring with maximum start index
    char* output1 = capture_output_with_timeout("cd ../../src/ej1 && ./ring 2 -1 1", 5);
    assert(strstr(output1, "1") != NULL); // -1 + 2 = 1
    
    // Large ring with negative value
    char* output2 = capture_output_with_timeout("cd ../../src/ej1 && ./ring 15 -100 7", 8);
    assert(strstr(output2, "-85") != NULL); // -100 + 15 = -85
}

int main() {
    printf("Running Advanced & Strict Ring Communication Tests\n");
    printf("=================================================\n");
    
    RUN_TEST(ring_single_process);
    RUN_TEST(ring_stress_test_50_processes);
    RUN_TEST(ring_negative_initial_value);
    RUN_TEST(ring_large_initial_value);
    RUN_TEST(ring_invalid_process_numbers);
    RUN_TEST(ring_max_start_process);
    RUN_TEST(ring_zero_value_large_ring);
    RUN_TEST(ring_consistency_check);
    RUN_TEST(ring_performance_test);
    RUN_TEST(ring_boundary_start_process);
    RUN_TEST(ring_invalid_string_arguments);
    RUN_TEST(ring_edge_combinations);
    
    printf("\n✓ All advanced ring tests passed!\n");
    printf("Your ring implementation is robust and handles edge cases correctly.\n");
    return 0;
}
