#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

// Advanced test framework for Ring Communication
#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    printf("Running advanced ring test: %s\n", #name); \
    test_##name(); \
    printf("✓ %s passed\n", #name); \
} while(0)

// Test: Ring handles negative values
TEST(ring_handles_negative_values) {
    system("cd ../../src/ej1 && make clean && make");
    
    FILE* fp = popen("cd ../../src/ej1 && ./ring 3 -5 0 2>/dev/null | tail -1", "r");
    char result[32];
    if (fgets(result, sizeof(result), fp)) {
        int final_result = atoi(result);
        assert(final_result == -2); // -5 + 3 = -2
    }
    pclose(fp);
}

// Test: Ring with single process (edge case)
TEST(ring_single_process) {
    system("cd ../../src/ej1 && make clean && make");
    
    FILE* fp = popen("cd ../../src/ej1 && ./ring 1 10 0 2>/dev/null | tail -1", "r");
    char result[32];
    if (fgets(result, sizeof(result), fp)) {
        int final_result = atoi(result);
        assert(final_result == 11); // 10 + 1 = 11
    }
    pclose(fp);
}

// Test: Ring with large initial value
TEST(ring_large_initial_value) {
    system("cd ../../src/ej1 && make clean && make");
    
    FILE* fp = popen("cd ../../src/ej1 && ./ring 5 1000 0 2>/dev/null | tail -1", "r");
    char result[32];
    if (fgets(result, sizeof(result), fp)) {
        int final_result = atoi(result);
        assert(final_result == 1005); // 1000 + 5 = 1005
    }
    pclose(fp);
}

// Test: Ring with maximum starting process
TEST(ring_max_start_process) {
    system("cd ../../src/ej1 && make clean && make");
    
    // Start from the last process (n-1)
    FILE* fp = popen("cd ../../src/ej1 && ./ring 4 50 3 2>/dev/null | tail -1", "r");
    char result[32];
    if (fgets(result, sizeof(result), fp)) {
        int final_result = atoi(result);
        assert(final_result == 54); // 50 + 4 = 54
    }
    pclose(fp);
}

// Test: Ring with zero initial value and large ring
TEST(ring_zero_value_large_ring) {
    system("cd ../../src/ej1 && make clean && make");
    
    FILE* fp = popen("cd ../../src/ej1 && ./ring 20 0 0 2>/dev/null | tail -1", "r");
    char result[32];
    if (fgets(result, sizeof(result), fp)) {
        int final_result = atoi(result);
        assert(final_result == 20); // 0 + 20 = 20
    }
    pclose(fp);
}

// Test: Ring performance with moderate number of processes
TEST(ring_performance_test) {
    system("cd ../../src/ej1 && make clean && make");
    
    FILE* fp = popen("cd ../../src/ej1 && ./ring 30 100 15 2>/dev/null | tail -1", "r");
    char result[32];
    if (fgets(result, sizeof(result), fp)) {
        int final_result = atoi(result);
        assert(final_result == 130); // 100 + 30 = 130
    }
    pclose(fp);
}

int main() {
    printf("Running Advanced Ring Communication Tests\n");
    printf("========================================\n");
    
    RUN_TEST(ring_handles_negative_values);
    RUN_TEST(ring_single_process);
    RUN_TEST(ring_large_initial_value);
    RUN_TEST(ring_max_start_process);
    RUN_TEST(ring_zero_value_large_ring);
    RUN_TEST(ring_performance_test);
    
    printf("\n✓ All advanced ring tests passed!\n");
    printf("Ring implementation handles edge cases correctly.\n");
    return 0;
}
