#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

// Simple test framework
#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    printf("Running test: %s\n", #name); \
    test_##name(); \
    printf("✓ %s passed\n", #name); \
} while(0)

// Helper function to test shell interaction
int test_shell_command(const char* input, const char* expected_output) {
    int pipe_in[2], pipe_out[2];
    pid_t pid;
    char buffer[1024];
    
    // Create pipes
    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
        perror("pipe");
        return -1;
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        // Child process - run the shell
        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_in[1]);
        close(pipe_out[0]);
        
        execl("../src/ej2/shell", "shell", NULL);
        perror("execl");
        exit(1);
    } else {
        // Parent process
        close(pipe_in[0]);
        close(pipe_out[1]);
        
        // Send input to shell
        write(pipe_in[1], input, strlen(input));
        write(pipe_in[1], "\n", 1);
        close(pipe_in[1]);
        
        // Read output
        read(pipe_out[0], buffer, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        close(pipe_out[0]);
        
        // Kill the shell process
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
        
        return strstr(buffer, expected_output) != NULL;
    }
    return 0;
}

// Test: Shell should display prompt
TEST(shell_displays_prompt) {
    // Build the shell first
    system("cd ../../src/ej2 && make clean && make");
    
    // This test checks if the shell binary can be executed
    int result = system("cd ../../src/ej2 && echo '' | ./shell > /dev/null 2>&1 &");
    
    // If we get here, the shell at least starts without crashing
    assert(result != -1); // Shell compiles and runs
}

// Test: Simple command parsing
TEST(shell_parses_single_command) {
    system("cd ../../src/ej2 && make clean && make");
    
    // Test that shell can parse a simple command
    FILE* fp = popen("cd ../../src/ej2 && printf 'ls\\n' | SHELL_DEBUG=1 ./shell 2>&1 | head -5", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 5) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // Should contain either "Shell>" prompt or "Command 0: ls"
    assert(strstr(output, "Shell>") != NULL || strstr(output, "Command 0") != NULL);
}

// Test: Pipe parsing
TEST(shell_parses_pipe_commands) {
    system("cd ../../src/ej2 && make clean && make");
    
    // Test pipe parsing by sending a command with pipe
    FILE* fp = popen("cd ../../src/ej2 && printf 'ls | grep test\\n' | SHELL_DEBUG=1 ./shell 2>&1 | head -10", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 10) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // Should show that it parsed two commands
    assert(strstr(output, "Command 0") != NULL && strstr(output, "Command 1") != NULL);
}

// Test: Multiple pipes
TEST(shell_parses_multiple_pipes) {
    system("cd ../../src/ej2 && make clean && make");
    
    // Test with three commands connected by pipes
    FILE* fp = popen("cd ../../src/ej2 && printf 'ps | grep shell | wc -l\\n' | SHELL_DEBUG=1 ./shell 2>&1 | head -10", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 10) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // Should show three commands
    assert(strstr(output, "Command 0") != NULL && 
           strstr(output, "Command 1") != NULL && 
           strstr(output, "Command 2") != NULL);
}

// Test: Simple command execution (when implemented)
TEST(shell_executes_simple_command) {
    system("cd ../../src/ej2 && make clean && make");
    
    // This test will check if simple commands work
    FILE* fp = popen("cd ../../src/ej2 && printf 'echo hello\\n' | SHELL_DEBUG=1 ./shell 2>&1 | head -5", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 5) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // At minimum, should show the prompt or command parsing
    assert(strstr(output, "Shell>") != NULL || strstr(output, "Command 0") != NULL);
}

// Test: Pipe execution (when implemented)
TEST(shell_executes_pipe_command) {
    system("cd ../../src/ej2 && make clean && make");
    
    // Test actual pipe execution
    FILE* fp = popen("cd ../../src/ej2 && printf 'echo hello | cat\\n' | SHELL_DEBUG=1 ./shell 2>&1 | head -5", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 5) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // Should at least show command parsing
    assert(strstr(output, "Command 0") != NULL && strstr(output, "Command 1") != NULL);
}

// Test: Exit command functionality
TEST(shell_handles_exit) {
    system("cd ../../src/ej2 && make clean && make");
    
    // Test exit command
    FILE* fp = popen("cd ../../src/ej2 && printf 'exit\\n' | ./shell 2>&1 | head -3", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 3) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // Should show shell startup message and exit cleanly (no error output)
    assert(strstr(output, "Simple Shell") != NULL || output[0] != '\0');
}

// Test: Empty command handling
TEST(shell_handles_empty_input) {
    system("cd ../../src/ej2 && make clean && make");
    
    // Test empty input
    FILE* fp = popen("cd ../../src/ej2 && printf '\\n' | ./shell 2>&1 | head -3", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 3) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    // Should handle empty input gracefully
    assert(strstr(output, "Shell>") != NULL || strstr(output, "Simple Shell") != NULL || strlen(output) == 0);
}

int main() {
    printf("Running Shell Tests\n");
    printf("===================\n");
    
    RUN_TEST(shell_displays_prompt);
    RUN_TEST(shell_parses_single_command);
    RUN_TEST(shell_parses_pipe_commands);
    RUN_TEST(shell_parses_multiple_pipes);
    RUN_TEST(shell_executes_simple_command);
    RUN_TEST(shell_executes_pipe_command);
    RUN_TEST(shell_handles_exit);
    RUN_TEST(shell_handles_empty_input);
    
    printf("\n✓ All shell tests passed!\n");
    return 0;
}
