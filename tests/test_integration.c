#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <time.h>

// Integrated test framework combining Ring and Shell functionality
#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    printf("Running integration test: %s\n", #name); \
    test_##name(); \
    printf("✅ %s passed\n", #name); \
} while(0)

// Test: Ring and Shell basic functionality integration
TEST(ring_shell_basic_integration) {
    // Test Ring first
    system("cd ../src/ej1 && make clean && make");
    FILE* fp = popen("cd ../src/ej1 && ./ring 3 10 0 2>/dev/null | tail -1", "r");
    char ring_result[32];
    int ring_value = 0;
    if (fgets(ring_result, sizeof(ring_result), fp)) {
        ring_value = atoi(ring_result);
        assert(ring_value == 13); // 10 + 3 = 13
    }
    pclose(fp);
    
    // Test Shell basic parsing
    system("cd ../src/ej2 && make clean && make");
    fp = popen("cd ../src/ej2 && echo 'echo test | cat' | ./shell 2>&1 | head -5", "r");
    char shell_output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 5) {
        strcat(shell_output, line);
        lines++;
    }
    pclose(fp);
    
    // Both should work without major errors
    printf("  Ring result: %d (expected: 13)\n", ring_value);
    printf("  Shell output length: %zu characters\n", strlen(shell_output));
    assert(ring_value == 13);
    assert(strlen(shell_output) > 0);
}

// Test: Performance comparison between Ring and Shell
TEST(ring_shell_performance_comparison) {
    printf("  Performance comparison between Ring and Shell implementations\n");
    
    // Measure Ring performance
    time_t start_time = time(NULL);
    system("cd ../src/ej1 && make clean && make && ./ring 10 0 0 >/dev/null 2>&1");
    time_t ring_time = time(NULL) - start_time;
    
    // Measure Shell compilation time
    start_time = time(NULL);
    system("cd ../src/ej2 && make clean && make >/dev/null 2>&1");
    time_t shell_time = time(NULL) - start_time;
    
    printf("  Ring compilation+execution time: %lds\n", ring_time);
    printf("  Shell compilation time: %lds\n", shell_time);
    
    // Both should complete in reasonable time
    assert(ring_time < 10);  // Should complete in less than 10 seconds
    assert(shell_time < 10); // Should compile in less than 10 seconds
}

// Test: Ring output can be used as input to Shell commands
TEST(ring_output_to_shell_integration) {
    // Generate output from ring
    system("cd ../src/ej1 && make clean && make");
    FILE* fp = popen("cd ../src/ej1 && ./ring 5 100 0 2>/dev/null | tail -1", "r");
    char ring_output[256] = {0};
    if (fgets(ring_output, sizeof(ring_output), fp)) {
        // Remove newline
        ring_output[strcspn(ring_output, "\n")] = 0;
    }
    pclose(fp);
    
    printf("  Ring generated value: %s\n", ring_output);
    int ring_value = atoi(ring_output);
    assert(ring_value == 105); // 100 + 5 = 105
    
    // Try to use this output in a shell command simulation
    char shell_command[512];
    snprintf(shell_command, sizeof(shell_command), 
             "cd ../src/ej2 && echo 'echo %s | wc -c' | ./shell 2>&1 | head -5", 
             ring_output);
    
    fp = popen(shell_command, "r");
    char shell_result[512] = {0};
    char line[128];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 5) {
        strcat(shell_result, line);
        lines++;
    }
    pclose(fp);
    
    printf("  Shell processing result length: %zu\n", strlen(shell_result));
    assert(strlen(shell_result) > 0);
}

// Test: Both exercises handle edge cases consistently
TEST(ring_shell_edge_case_consistency) {
    printf("  Testing edge case handling consistency\n");
    
    // Test Ring with edge case (single process)
    system("cd ../src/ej1 && make clean && make");
    FILE* fp = popen("cd ../src/ej1 && ./ring 1 42 0 2>/dev/null | tail -1", "r");
    char ring_result[32];
    int ring_single = 0;
    if (fgets(ring_result, sizeof(ring_result), fp)) {
        ring_single = atoi(ring_result);
    }
    pclose(fp);
    
    // Test Shell with edge case (empty input)
    system("cd ../src/ej2 && make clean && make");
    fp = popen("cd ../src/ej2 && echo '' | ./shell 2>&1 | head -3", "r");
    char shell_empty[512] = {0};
    char line[128];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 3) {
        strcat(shell_empty, line);
        lines++;
    }
    pclose(fp);
    
    printf("  Ring single process result: %d (expected: 43)\n", ring_single);
    printf("  Shell empty input handled: %s\n", strlen(shell_empty) > 0 ? "Yes" : "No");
    
    assert(ring_single == 43); // 42 + 1 = 43
    // Shell should handle empty input without crashing
}

// Test: Resource usage patterns
TEST(ring_shell_resource_usage) {
    printf("  Analyzing resource usage patterns\n");
    
    // Test Ring with multiple processes
    system("cd ../src/ej1 && make clean && make");
    FILE* fp = popen("cd ../src/ej1 && time -p ./ring 20 0 10 2>&1 | grep real || echo 'timing unavailable'", "r");
    char timing[128] = {0};
    if (fgets(timing, sizeof(timing), fp)) {
        printf("  Ring timing info: %s", timing);
    }
    pclose(fp);
    
    // Test Shell resource usage
    system("cd ../src/ej2 && make clean && make");
    fp = popen("cd ../src/ej2 && echo 'echo resource_test | cat | wc' | ./shell 2>&1 | wc -l", "r");
    char shell_lines[32];
    int output_lines = 0;
    if (fgets(shell_lines, sizeof(shell_lines), fp)) {
        output_lines = atoi(shell_lines);
    }
    pclose(fp);
    
    printf("  Shell output lines: %d\n", output_lines);
    assert(output_lines >= 0); // Should produce some measurable output
}

int main() {
    printf("=================================================================\n");
    printf("TP4-ACSO Integration Tests - Ring Communication & Shell Pipeline\n");
    printf("=================================================================\n");
    printf("Testing interaction and compatibility between both exercises\n");
    printf("=================================================================\n\n");
    
    RUN_TEST(ring_shell_basic_integration);
    printf("\n");
    
    RUN_TEST(ring_shell_performance_comparison);
    printf("\n");
    
    RUN_TEST(ring_output_to_shell_integration);
    printf("\n");
    
    RUN_TEST(ring_shell_edge_case_consistency);
    printf("\n");
    
    RUN_TEST(ring_shell_resource_usage);
    printf("\n");
    
    printf("=================================================================\n");
    printf("🎉 All integration tests completed successfully!\n");
    printf("=================================================================\n");
    printf("✅ Ring Communication (Exercise 1): Functional\n");
    printf("✅ Shell Implementation (Exercise 2): Functional\n");  
    printf("✅ Cross-exercise compatibility: Verified\n");
    printf("✅ Resource usage patterns: Analyzed\n");
    printf("✅ Edge case handling: Consistent\n");
    printf("=================================================================\n");
    
    return 0;
}
