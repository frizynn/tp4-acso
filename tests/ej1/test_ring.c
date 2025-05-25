#define _GNU_SOURCE
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>

// Simple test framework
#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    printf("Running test: %s\n", #name); \
    test_##name(); \
    printf("✓ %s passed\n", #name); \
} while(0)

// Helper function to capture program output
char* capture_output(const char* command) {
    FILE* fp;
    char path[1035];
    static char result[4096] = {0};
    
    fp = popen(command, "r");
    if (fp == NULL) {
        return NULL;
    }
    
    result[0] = '\0';
    while (fgets(path, sizeof(path), fp) != NULL) {
        strcat(result, path);
    }
    
    pclose(fp);
    return result;
}

// Test: Program should accept correct number of arguments
TEST(ring_accepts_correct_arguments) {
    // Build the program first
    system("cd ../../src/ej1 && make clean && make");
    
    char* output = capture_output("cd ../../src/ej1 && ./ring 2>&1 | head -1");
    assert(strstr(output, "Uso: anillo <n> <c> <s>") != NULL);
}

// Test: Ring with 3 processes, initial value 5, starting from process 0
TEST(ring_basic_functionality_3_processes) {
    system("cd ../../src/ej1 && make clean && make");
    
    char* output = capture_output("cd ../../src/ej1 && ./ring 3 5 0");
    
    // Should create 3 processes, send value 5, and get result 8 (5+3)
    assert(strstr(output, "Se crearán 3 procesos") != NULL);
    assert(strstr(output, "enviará el caracter 5") != NULL);
    assert(strstr(output, "desde proceso 0") != NULL);
    
    // The final result should be initial value + number of processes
    // With 3 processes: 5 -> 6 -> 7 -> 8
    assert(strstr(output, "8") != NULL);
}

// Test: Ring with 4 processes, initial value 10, starting from process 1
TEST(ring_different_start_process) {
    system("cd ../../src/ej1 && make clean && make");
    
    char* output = capture_output("cd ../../src/ej1 && ./ring 4 10 1");
    
    // Should work with different starting process
    assert(strstr(output, "Se crearán 4 procesos") != NULL);
    assert(strstr(output, "enviará el caracter 10") != NULL);
    assert(strstr(output, "desde proceso 1") != NULL);
    
    // Final result: 10 + 4 = 14
    assert(strstr(output, "14") != NULL);
}

// Test: Ring with 5 processes, initial value 0
TEST(ring_with_zero_value) {
    system("cd ../../src/ej1 && make clean && make");
    
    char* output = capture_output("cd ../../src/ej1 && ./ring 5 0 0");
    
    // Final result: 0 + 5 = 5
    assert(strstr(output, "5") != NULL);
}

// Test: Ring with large number of processes
TEST(ring_large_number_processes) {
    system("cd ../../src/ej1 && make clean && make");
    
    char* output = capture_output("cd ../../src/ej1 && ./ring 10 100 0");
    
    // Final result: 100 + 10 = 110
    assert(strstr(output, "110") != NULL);
}

// Test: Invalid arguments
TEST(ring_invalid_arguments) {
    system("cd ../../src/ej1 && make clean && make");
    
    // Test with too few arguments
    char* output1 = capture_output("cd ../../src/ej1 && ./ring 2>&1");
    assert(strstr(output1, "Uso: anillo <n> <c> <s>") != NULL);
    
    // Test with too many arguments
    char* output2 = capture_output("cd ../../src/ej1 && ./ring 1 2 3 4 2>&1");
    assert(strstr(output2, "Uso: anillo <n> <c> <s>") != NULL);
}

// Test: Edge case - minimum ring size (2 processes)
TEST(ring_minimum_size) {
    system("cd ../../src/ej1 && make clean && make");
    
    char* output = capture_output("cd ../../src/ej1 && ./ring 2 7 0");
    
    // Final result: 7 + 2 = 9
    assert(strstr(output, "9") != NULL);
}

int main() {
    printf("Running Ring Communication Tests\n");
    printf("================================\n");
    
    RUN_TEST(ring_accepts_correct_arguments);
    RUN_TEST(ring_basic_functionality_3_processes);
    RUN_TEST(ring_different_start_process);
    RUN_TEST(ring_with_zero_value);
    RUN_TEST(ring_large_number_processes);
    RUN_TEST(ring_invalid_arguments);
    RUN_TEST(ring_minimum_size);
    
    printf("\n✓ All ring tests passed!\n");
    return 0;
}
