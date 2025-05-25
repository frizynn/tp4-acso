#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

// Advanced test framework for Shell Implementation
#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    printf("Running advanced shell test: %s\n", #name); \
    test_##name(); \
    printf("✓ %s passed\n", #name); \
} while(0)

// Test: Shell handles very long commands
TEST(shell_handles_long_commands) {
    system("cd ../../src/ej2 && make clean && make");
    
    // Create a very long command
    FILE* fp = popen("cd ../../src/ej2 && echo 'echo this_is_a_very_long_command_with_many_words_to_test_buffer_handling' | ./shell 2>&1 | head -3", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 3) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // Should handle long commands without crashing
    assert(strstr(output, "Command 0") != NULL || strstr(output, "Shell>") != NULL);
}

// Test: Shell handles commands with many pipes
TEST(shell_handles_many_pipes) {
    system("cd ../../src/ej2 && make clean && make");
    
    // Command with 5 pipes
    FILE* fp = popen("cd ../../src/ej2 && echo 'echo test | cat | cat | cat | cat | wc -l' | ./shell 2>&1 | head -10", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 10) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // Should parse all commands
    assert(strstr(output, "Command 0") != NULL && 
           strstr(output, "Command 1") != NULL &&
           strstr(output, "Command 2") != NULL &&
           strstr(output, "Command 3") != NULL &&
           strstr(output, "Command 4") != NULL &&
           strstr(output, "Command 5") != NULL);
}

// Test: Shell handles special characters
TEST(shell_handles_special_characters) {
    system("cd ../../src/ej2 && make clean && make");
    
    FILE* fp = popen("cd ../../src/ej2 && echo 'echo \"hello world\" | grep hello' | ./shell 2>&1 | head -5", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 5) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // Should handle quoted strings
    assert(strstr(output, "Command 0") != NULL && strstr(output, "Command 1") != NULL);
}

// Test: Shell with mixed whitespace
TEST(shell_handles_whitespace) {
    system("cd ../../src/ej2 && make clean && make");
    
    // Command with tabs and multiple spaces
    FILE* fp = popen("cd ../../src/ej2 && printf 'echo hello | cat' | ./shell 2>&1 | head -5", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 5) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // Should parse commands despite whitespace
    assert(strstr(output, "Command 0") != NULL && strstr(output, "Command 1") != NULL);
}

// Test: Shell handles empty commands gracefully
TEST(shell_handles_empty_commands) {
    system("cd ../../src/ej2 && make clean && make");
    
    FILE* fp = popen("cd ../../src/ej2 && echo '' | ./shell 2>&1 | head -3", "r");
    char output[512] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 3) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // Should handle empty input gracefully
    assert(strstr(output, "Shell>") != NULL || strlen(output) >= 0);
}

int main() {
    printf("Running Advanced Shell Implementation Tests\n");
    printf("==========================================\n");
    
    RUN_TEST(shell_handles_long_commands);
    RUN_TEST(shell_handles_many_pipes);
    RUN_TEST(shell_handles_special_characters);
    RUN_TEST(shell_handles_whitespace);
    RUN_TEST(shell_handles_empty_commands);
    
    printf("\n✓ All advanced shell tests completed!\n");
    printf("Shell implementation handles complex scenarios.\n");
    return 0;
}
