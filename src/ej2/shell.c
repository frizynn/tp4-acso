#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>

/* Configuration constants */
#define MAX_COMMANDS 200
#define MAX_ARGS 64 // TODO: see if it works properly with tests
#define COMMAND_BUFFER_SIZE 1024
#define LINE_BUFFER_SIZE 256

/* Return codes */
#define SUCCESS 0
#define ERROR_GENERAL 1
#define ERROR_MEMORY 2
#define ERROR_PIPE 3
#define ERROR_FORK 4

/* Global variables for signal handling */
static volatile sig_atomic_t g_shell_running = 1;

/**
 * Signal handler for graceful shutdown
 * Handles SIGINT (Ctrl+C) and SIGTERM for clean exit
 */
static void signal_handler(int sig) {
    (void)sig; // Suppress unused parameter warning
    g_shell_running = 0;
    write(STDOUT_FILENO, "\nShell shutting down...\n", 24);
}

/**
 * Setup signal handlers for graceful shutdown
 */
static void setup_signal_handlers(void) {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

/**
 * Safe string duplication with error checking
 * @param str String to duplicate
 * @return Duplicated string or NULL on failure
 */
static char* safe_strdup(const char* str) {
    if (!str) return NULL;
    
    char* dup = strdup(str);
    if (!dup) {
        fprintf(stderr, "Error: Memory allocation failed in strdup\n");
    }
    return dup;
}

/**
 * Trim whitespace from both ends of a string (in-place)
 * @param str String to trim (modified in place)
 * @return Pointer to trimmed string
 */
char* trim(char* str) {
    if (!str) return NULL;
    
    char *end;
    
    // Trim leading space
    while(isspace((unsigned char)*str)) str++;
    
    if(*str == 0)  // All spaces?
        return str;
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator character
    end[1] = '\0';
    
    return str;
}

/**
 * Parse command line into arguments with quote handling and bounds checking
 * This function properly handles:
 * - Arguments without quotes: ls | grep .zip
 * - Arguments with quotes: ls | grep ".zip"
 * - Arguments with spaces inside quotes: ls | grep ".png .zip"
 * - Error detection for unclosed quotes
 * 
 * @param command Command string to parse
 * @param args Array to store argument pointers
 * @return Number of arguments parsed, or -1 on error, -2 on unclosed quotes
 */
int parse_args(char* command, char** args) {
    if (!command || !args) return -1;
    
    int argc = 0;
    char* ptr = command;
    char* start;
    
    // Skip leading whitespace
    while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n')) {
        ptr++;
    }
    
    // Parse arguments up to MAX_ARGS (leaving space for NULL terminator)
    while (*ptr && argc < MAX_ARGS) {
        start = ptr;
        
        // Handle quoted arguments
        if (*ptr == '"') {
            ptr++; // Skip opening quote
            start = ptr; // Start after the quote
            
            // Find closing quote
            while (*ptr && *ptr != '"') {
                ptr++;
            }
            
            if (*ptr == '"') {
                *ptr = '\0'; // Null-terminate the argument (remove closing quote)
                ptr++; // Move past the closing quote
            } else {
                // Unclosed quote - this is an error
                return -2;
            }
        } else {
            // Handle unquoted arguments
            while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr != '"') {
                ptr++;
            }
            
            if (*ptr) {
                *ptr = '\0'; // Null-terminate
                ptr++;
            }
        }
        
        // Store the argument if it's not empty
        if (strlen(start) > 0) {
            args[argc++] = start;
        }
        
        // Skip whitespace before next argument
        while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n')) {
            ptr++;
        }
    }
    
    // Check if there are more arguments that couldn't be processed
    // Skip any remaining whitespace to see if there are more arguments
    while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n')) {
        ptr++;
    }
    if (*ptr) {
        // There are more arguments that couldn't be processed - overflow
        return -1;
    }
    
    args[argc] = NULL;
    return argc;
}

/**
 * Execute a single command with proper error handling
 * @param args Null-terminated array of command arguments
 * @return SUCCESS on success, error code on failure
 */
int execute_command(char** args) {
    if (!args || !args[0]) {
        return ERROR_GENERAL; // Empty command
    }
    
    // Handle built-in commands
    if (strcmp(args[0], "exit") == 0) {
        g_shell_running = 0;
        return SUCCESS;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) { 
            fprintf(stderr, "Error executing '%s': %s\n", args[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else if (pid > 0) {
        // Parent process
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return ERROR_GENERAL;
        }
        return WEXITSTATUS(status);
    } else {
        perror("fork");
        return ERROR_FORK;
    }
    return SUCCESS;
}

/**
 * Cleanup pipes in case of error
 * @param pipes Array of pipe file descriptors
 * @param num_pipes Number of pipes to clean up
 */
static void cleanup_pipes(int pipes[][2], int num_pipes) {
    for (int i = 0; i < num_pipes; i++) {
        if (pipes[i][0] != -1) close(pipes[i][0]);
        if (pipes[i][1] != -1) close(pipes[i][1]);
    }
}

/**
 * Wait for all valid child processes and handle cleanup
 * @param pids Array of process IDs
 * @param num_processes Number of processes
 * @return SUCCESS or error code
 */
static int wait_for_children(pid_t* pids, int num_processes) {
    int exit_status = SUCCESS;
    
    for (int i = 0; i < num_processes; i++) {
        if (pids[i] > 0) {  // Only wait for valid PIDs
            int status;
            if (waitpid(pids[i], &status, 0) == -1) {
                perror("waitpid");
                exit_status = ERROR_GENERAL;
            } else if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                exit_status = ERROR_GENERAL;
            }
        }
    }
    return exit_status;
}

/**
 * Execute commands connected by pipes with robust error handling
 * @param commands Array of command strings
 * @param num_commands Number of commands in pipeline
 * @return SUCCESS on success, error code on failure
 */
int execute_pipe(char** commands, int num_commands) {
    if (!commands || num_commands <= 0) {
        return ERROR_GENERAL;
    }
    
    // Allocate and initialize arrays
    int pipes[num_commands - 1][2];
    pid_t* pids = calloc(num_commands, sizeof(pid_t));
    char** cmd_copies = calloc(num_commands, sizeof(char*));
    
    if (!pids || !cmd_copies) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(pids);
        free(cmd_copies);
        return ERROR_MEMORY;
    }
    
    // Initialize arrays
    for (int i = 0; i < num_commands; i++) {
        pids[i] = -1;
        if (i < num_commands - 1) {
            pipes[i][0] = pipes[i][1] = -1;
        }
    }
    
    // Create all pipes first
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            cleanup_pipes(pipes, i);  // Clean up pipes created so far
            goto cleanup_and_exit;
        }
    }
    
    // Create copies of command strings for parsing
    for (int i = 0; i < num_commands; i++) {
        cmd_copies[i] = safe_strdup(commands[i]);
        if (!cmd_copies[i]) {
            goto cleanup_and_exit;
        }
    }
    
    // Create child processes
    for (int i = 0; i < num_commands; i++) {
        pids[i] = fork();
        
        if (pids[i] == -1) {
            perror("fork");
            goto cleanup_and_exit;
        }
        
        if (pids[i] == 0) {
            // Child process i
            char* args[MAX_ARGS];
            
            // Parse the command arguments
            int parse_result = parse_args(cmd_copies[i], args);
            if (parse_result <= 0) {
                if (parse_result == -1) {
                    fprintf(stderr, "Error: Too many arguments in command '%s' (maximum %d)\n", cmd_copies[i], MAX_ARGS);
                } else if (parse_result == -2) {
                    fprintf(stderr, "Error: Unclosed quotes in command '%s'\n", cmd_copies[i]);
                } else {
                    fprintf(stderr, "Error: Invalid command '%s'\n", cmd_copies[i]);
                }
                exit(EXIT_FAILURE);
            }
            
            // Handle built-in commands in pipeline
            if (strcmp(args[0], "exit") == 0) {
                // Exit command in pipeline - just exit silently
                exit(EXIT_SUCCESS);
            }
            
            // Set up pipes for this command
            if (i > 0) {
                // Not the first command: read from previous pipe
                if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
                    perror("dup2 stdin");
                    exit(EXIT_FAILURE);
                }
            }
            
            if (i < num_commands - 1) {
                // Not the last command: write to next pipe
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2 stdout");
                    exit(EXIT_FAILURE);
                }
            }
            
            // Close all pipe ends in child
            for (int j = 0; j < num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // For long pipelines, disable buffering to improve flow
            if (num_commands > 10) {
                setvbuf(stdout, NULL, _IONBF, 0);
                setvbuf(stdin, NULL, _IONBF, 0);
            }
            
            // Execute the command
            if (execvp(args[0], args) == -1) {
                fprintf(stderr, "Error executing '%s': %s\n", args[0], strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        
        // In parent: close pipes that are no longer needed as we create processes
        // This ensures proper data flow in long pipelines
        if (i > 0) {
            // Close the read end of the previous pipe since the current process is now reading from it
            close(pipes[i-1][0]);
        }
        if (i < num_commands - 1) {
            // Close the write end of the current pipe since the current process is now writing to it
            close(pipes[i][1]);
        }
    }
    
    // Parent process: close any remaining pipe ends
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipes[i][0] != -1) close(pipes[i][0]);
        if (pipes[i][1] != -1) close(pipes[i][1]);
    }
    
    // Wait for all children
    int result = wait_for_children(pids, num_commands);
    
    cleanup_and_exit:
    // Cleanup
    if (cmd_copies) {
        for (int i = 0; i < num_commands; i++) {
            free(cmd_copies[i]);
        }
        free(cmd_copies);
    }
    free(pids);
    
    return result;
}

/**
 * Parse a full command line into separate commands divided by pipes
 * This function properly handles quotes so that pipes inside quotes are not treated as separators
 * @param line Input command line
 * @param commands Array to store command strings
 * @return Number of commands found, or -1 on error
 */
int parse_pipe_commands(char* line, char** commands) {
    if (!line || !commands) return -1;
    
    int num_commands = 0;
    char* start = line;
    char* ptr = line;
    int in_quotes = 0;
    
    while (*ptr && num_commands < MAX_COMMANDS - 1) {
        if (*ptr == '"' && (ptr == line || *(ptr-1) != '\\')) {
            in_quotes = !in_quotes;
        } else if (*ptr == '|' && !in_quotes) {
            // Found a pipe separator outside of quotes
            *ptr = '\0';  // Null-terminate the current command
            commands[num_commands] = trim(start);
            
            // Debug output if SHELL_DEBUG is set
            if (getenv("SHELL_DEBUG")) {
                printf("Command %d: %s\n", num_commands, commands[num_commands]);
                fflush(stdout);
            }
            
            num_commands++;
            start = ptr + 1;  // Start of next command
        }
        ptr++;
    }
    
    // Add the last command
    if (start && num_commands < MAX_COMMANDS - 1) {
        commands[num_commands] = trim(start);
        
        // Debug output if SHELL_DEBUG is set
        if (getenv("SHELL_DEBUG")) {
            printf("Command %d: %s\n", num_commands, commands[num_commands]);
            fflush(stdout);
        }
        
        num_commands++;
    }
    
    commands[num_commands] = NULL;
    return num_commands;
}

/**
 * Main shell loop with interactive prompt
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main() {
    char* line = NULL;
    size_t line_size = 0;
    ssize_t line_length;
    
    // Detect if we're running interactively
    int is_interactive = isatty(STDIN_FILENO);
    
    // Setup signal handlers
    setup_signal_handlers();
    
    // Only show welcome message in interactive mode
    if (is_interactive) {
        printf("Shell started. Type 'exit' to quit.\n");
    }
    
    while (g_shell_running) {
        // Only show prompt in interactive mode
        if (is_interactive) {
            printf("Shell> ");
            fflush(stdout);
        }
        
        // Read input line
        line_length = getline(&line, &line_size, stdin);
        
        if (line_length == -1) {
            if (feof(stdin)) {
                if (is_interactive) {
                    printf("\nGoodbye!\n");
                }
                break;
            } else {
                perror("getline");
                continue;
            }
        }
        
        // Remove trailing newline
        if (line_length > 0 && line[line_length - 1] == '\n') {
            line[line_length - 1] = '\0';
        }
        
        // Skip empty lines
        char* trimmed_line = trim(line);
        if (strlen(trimmed_line) == 0) {
            continue;
        }
        
        // Create a copy for parsing
        char* line_copy = safe_strdup(trimmed_line);
        if (!line_copy) {
            continue;
        }
        
        // Parse pipe commands
        char* commands[MAX_COMMANDS];
        int num_commands = parse_pipe_commands(line_copy, commands);
        
        if (num_commands > 0) {
            if (num_commands == 1) {
                // Single command
                char* args[MAX_ARGS];
                char* cmd_copy = safe_strdup(commands[0]);
                if (cmd_copy) {
                    int parse_result = parse_args(cmd_copy, args);
                    if (parse_result > 0) {
                        execute_command(args);
                    } else if (parse_result == -1) {
                        fprintf(stderr, "Error: Too many arguments in command '%s' (maximum %d)\n", cmd_copy, MAX_ARGS);
                    } else if (parse_result == -2) {
                        fprintf(stderr, "Error: Unclosed quotes\n");
                    } else {
                        fprintf(stderr, "Error: Invalid command\n");
                    }
                    free(cmd_copy);
                }
            } else {
                // Pipeline
                execute_pipe(commands, num_commands);
            }
        }
        
        free(line_copy);
        
        // In non-interactive mode, exit after processing one command sequence
        if (!is_interactive) {
            break;
        }
    }
    
    free(line);
    if (is_interactive) {
        printf("Shell terminated.\n");
    }
    return EXIT_SUCCESS;
}