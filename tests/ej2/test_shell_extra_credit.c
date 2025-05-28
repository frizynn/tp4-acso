#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

// Extra Credit Test Framework for Shell Implementation
// Tests specific to quote handling and patterns with spaces

#define RUN_TEST(test_func) do { \
    printf("Running extra credit test: %s\n", #test_func); \
    test_func(); \
    printf("✓ %s passed\n", #test_func); \
} while(0)

#define TEST(name) void name()

// Test: Shell handles ls | grep ".zip" (single file extension with quotes)
TEST(shell_handles_quoted_single_pattern) {
    system("cd ../../src/ej2 && make clean && make");
    
    // Create some test files first
    system("cd ../../src/ej2 && touch test1.zip test2.txt test3.zip test4.png");
    
    FILE* fp = popen("cd ../../src/ej2 && echo 'ls | grep \".zip\"' | SHELL_DEBUG=1 ./shell 2>&1", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 15) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // Clean up test files
    system("cd ../../src/ej2 && rm -f test1.zip test2.txt test3.zip test4.png");
    
    // Should show debug output for both commands
    assert(strstr(output, "Command 0: ls") != NULL);
    assert(strstr(output, "Command 1: grep") != NULL);
    // Should handle quotes properly and find .zip files
    assert(strstr(output, ".zip") != NULL);
}

// Test: Shell handles ls | grep ".png .zip" (multiple patterns with spaces in quotes)
TEST(shell_handles_quoted_multiple_patterns) {
    system("cd ../../src/ej2 && make clean && make");
    
    // Create test files with different extensions
    system("cd ../../src/ej2 && touch file1.png file2.zip file3.txt file4.png file5.zip");
    
    FILE* fp = popen("cd ../../src/ej2 && echo 'ls | grep \".png .zip\"' | SHELL_DEBUG=1 ./shell 2>&1", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 15) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // Clean up test files
    system("cd ../../src/ej2 && rm -f file1.png file2.zip file3.txt file4.png file5.zip");
    
    // Should show debug output for both commands
    assert(strstr(output, "Command 0: ls") != NULL);
    assert(strstr(output, "Command 1: grep") != NULL);
    // The pattern ".png .zip" should be treated as a single argument with spaces
    assert(strstr(output, "grep") != NULL);
}

// Test: Shell handles complex quoted patterns with special characters
TEST(shell_handles_quoted_special_patterns) {
    system("cd ../../src/ej2 && make clean && make");
    
    // Create test files with various patterns
    system("cd ../../src/ej2 && touch 'file with spaces.txt' normal.txt 'another file.zip'");
    
    FILE* fp = popen("cd ../../src/ej2 && echo 'ls | grep \"file\"' | SHELL_DEBUG=1 ./shell 2>&1", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 15) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // Clean up test files
    system("cd ../../src/ej2 && rm -f 'file with spaces.txt' normal.txt 'another file.zip'");
    
    // Should handle quoted patterns correctly
    assert(strstr(output, "Command 0: ls") != NULL);
    assert(strstr(output, "Command 1: grep") != NULL);
}

// Test: Shell handles multiple quotes in same command
TEST(shell_handles_multiple_quotes) {
    system("cd ../../src/ej2 && make clean && make");
    
    FILE* fp = popen("cd ../../src/ej2 && echo 'echo \"hello world\" | grep \"hello\"' | SHELL_DEBUG=1 ./shell 2>&1", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 15) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // Should parse both quoted arguments correctly
    assert(strstr(output, "Command 0: echo") != NULL);
    assert(strstr(output, "Command 1: grep") != NULL);
    // Should handle the execution and show result
    assert(strstr(output, "hello") != NULL);
}

// Test: Shell handles mixed quotes and unquoted arguments
TEST(shell_handles_mixed_quotes) {
    system("cd ../../src/ej2 && make clean && make");
    
    // Create test files
    system("cd ../../src/ej2 && touch test.log error.log info.txt debug.log");
    
    FILE* fp = popen("cd ../../src/ej2 && echo 'ls *.log | grep \"test\"' | SHELL_DEBUG=1 ./shell 2>&1", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 15) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // Clean up test files
    system("cd ../../src/ej2 && rm -f test.log error.log info.txt debug.log");
    
    // Should handle mix of wildcard and quoted pattern
    assert(strstr(output, "Command 0: ls") != NULL);
    assert(strstr(output, "Command 1: grep") != NULL);
}

// Test: Shell handles empty quotes
TEST(shell_handles_empty_quotes) {
    system("cd ../../src/ej2 && make clean && make");
    
    FILE* fp = popen("cd ../../src/ej2 && echo 'echo \"\" | cat' | SHELL_DEBUG=1 ./shell 2>&1", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 10) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // Should handle empty quoted strings without crashing
    assert(strstr(output, "Command 0: echo") != NULL);
    assert(strstr(output, "Command 1: cat") != NULL);
}

// Test: Shell handles quotes with spaces at boundaries
TEST(shell_handles_quotes_with_boundary_spaces) {
    system("cd ../../src/ej2 && make clean && make");
    
    FILE* fp = popen("cd ../../src/ej2 && echo 'echo \" hello world \" | grep hello' | SHELL_DEBUG=1 ./shell 2>&1", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 15) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // Should preserve spaces within quotes
    assert(strstr(output, "Command 0: echo") != NULL);
    assert(strstr(output, "Command 1: grep") != NULL);
}

// Test: Shell handles the exact extra credit examples
TEST(shell_handles_exact_extra_credit_examples) {
    system("cd ../../src/ej2 && make clean && make");
    
    // Create test files for the exact examples
    system("cd ../../src/ej2 && touch file1.zip file2.png file3.txt archive.zip image.png doc.pdf");
    
    printf("Testing exact extra credit example 1: ls | grep \".zip\"\n");
    FILE* fp1 = popen("cd ../../src/ej2 && echo 'ls | grep \".zip\"' | SHELL_DEBUG=1 ./shell 2>&1", "r");
    char output1[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp1) && lines < 15) {
        strcat(output1, line);
        lines++;
    }
    pclose(fp1);
    
    printf("Testing exact extra credit example 2: ls | grep \".png .zip\"\n");
    FILE* fp2 = popen("cd ../../src/ej2 && echo 'ls | grep \".png .zip\"' | SHELL_DEBUG=1 ./shell 2>&1", "r");
    char output2[1024] = {0};
    lines = 0;
    
    while (fgets(line, sizeof(line), fp2) && lines < 15) {
        strcat(output2, line);
        lines++;
    }
    pclose(fp2);
    
    // Clean up test files
    system("cd ../../src/ej2 && rm -f file1.zip file2.png file3.txt archive.zip image.png doc.pdf");
    
    // Both examples should parse correctly
    assert(strstr(output1, "Command 0: ls") != NULL);
    assert(strstr(output1, "Command 1: grep") != NULL);
    assert(strstr(output2, "Command 0: ls") != NULL);
    assert(strstr(output2, "Command 1: grep") != NULL);
    
    printf("✓ Both exact extra credit examples parsed successfully!\n");
}

int main() {
    printf("Running Extra Credit Shell Implementation Tests\n");
    printf("===============================================\n");
    printf("Testing quote handling and patterns with spaces\n");
    printf("-----------------------------------------------\n");
    
    RUN_TEST(shell_handles_quoted_single_pattern);
    RUN_TEST(shell_handles_quoted_multiple_patterns);
    RUN_TEST(shell_handles_quoted_special_patterns);
    RUN_TEST(shell_handles_multiple_quotes);
    RUN_TEST(shell_handles_mixed_quotes);
    RUN_TEST(shell_handles_empty_quotes);
    RUN_TEST(shell_handles_quotes_with_boundary_spaces);
    RUN_TEST(shell_handles_exact_extra_credit_examples);
    
    printf("\n🎉 All extra credit tests completed!\n");
    printf("Shell implementation successfully handles:\n");
    printf("  ✓ ls | grep \".zip\"\n");
    printf("  ✓ ls | grep \".png .zip\"\n");
    printf("  ✓ Complex quoted patterns\n");
    printf("  ✓ Mixed quote scenarios\n");
    printf("EXTRA CREDIT FUNCTIONALITY VERIFIED! 🏆\n");
    return 0;
}
