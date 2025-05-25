/*
 * TP4 - Ejercicio 2: Professional Shell Implementation with Pipe Support
 * 
 * This shell implementation provides:
 * - Interactive command prompt with robust error handling
 * - Single command execution with proper cleanup
 * - Pipe command chaining (command1 | command2 | ...)
 * - Built-in commands (exit)
 * - Memory leak prevention and signal handling
 * - Professional error handling and resource management
 * 
 * Author: TP4-ACSO Assignment
 * Date: May 2025
 * Version: 2.0 - Professional Edition
 */

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
#define MAX_ARGS 64
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
 * Parse command line into arguments with bounds checking
 * @param command Command string to parse (will be modified)
 * @param args Array to store argument pointers
 * @return Number of arguments parsed, or -1 on error
 */
int parse_args(char* command, char** args) {
    if (!command || !args) return -1;
    
    int argc = 0;
    char* token = strtok(command, " \t\n");
    
    while (token != NULL && argc < MAX_ARGS - 1) {
        args[argc++] = token;
        token = strtok(NULL, " \t\n");
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
    
    // Create child processes for each command
    for (int i = 0; i < num_commands; i++) {
        char* args[MAX_ARGS];
        
        // Create a copy of the command for parsing
        cmd_copies[i] = safe_strdup(commands[i]);
        if (!cmd_copies[i]) {
            goto cleanup_and_exit;
        }
        
        trim(cmd_copies[i]);
        
        // Skip empty commands
        if (parse_args(cmd_copies[i], args) <= 0) {
            continue;
        }
        
        pids[i] = fork();
        
        if (pids[i] == 0) {
            // Child process
            
            // Set up input redirection
            if (i > 0) {
                if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
                    perror("dup2 input");
                    exit(EXIT_FAILURE);
                }
            }
            
            // Set up output redirection
            if (i < num_commands - 1) {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2 output");
                    exit(EXIT_FAILURE);
                }
            }
            
            // Close all pipe file descriptors in child
            cleanup_pipes(pipes, num_commands - 1);
            
            // Execute the command
            if (execvp(args[0], args) == -1) {
                fprintf(stderr, "Error executing '%s': %s\n", args[0], strerror(errno));
                exit(EXIT_FAILURE);
            }
        } else if (pids[i] == -1) {
            perror("fork");
            goto cleanup_and_exit;
        }
    }
    
    // Close all pipe file descriptors in parent
    cleanup_pipes(pipes, num_commands - 1);
    
    // Wait for all child processes
    int result = wait_for_children(pids, num_commands);
    
    // Cleanup allocated memory
    for (int i = 0; i < num_commands; i++) {
        free(cmd_copies[i]);
    }
    free(cmd_copies);
    free(pids);
    
    return result;

cleanup_and_exit:
    // Error cleanup path
    cleanup_pipes(pipes, num_commands - 1);
    
    // Kill any processes that were started
    for (int i = 0; i < num_commands; i++) {
        if (pids[i] > 0) {
            kill(pids[i], SIGTERM);
            waitpid(pids[i], NULL, 0);
        }
    }
    
    // Free allocated memory
    for (int i = 0; i < num_commands; i++) {
        free(cmd_copies[i]);
    }
    free(cmd_copies);
    free(pids);
    
    return ERROR_GENERAL;
}

/**
 * Cleanup allocated commands array
 * @param commands Array of command strings
 * @param count Number of commands to free
 */
static void cleanup_commands(char** commands, int count) {
    for (int i = 0; i < count; i++) {
        free(commands[i]);
    }
}

/**
 * Main shell function with robust error handling and memory management
 */
int main() {
    char command[COMMAND_BUFFER_SIZE];
    char *commands[MAX_COMMANDS];
    int command_count;
    
    // Setup signal handlers for graceful shutdown
    setup_signal_handlers();
    
    printf("Simple Shell with Pipe Support\n");
    printf("Type 'exit' to quit\n\n");

    /* Main shell loop */
    while (g_shell_running) {
        /* Display prompt */
        printf("Shell> ");
        fflush(stdout); // Ensure prompt is displayed immediately
        
        /* Read user input with bounds checking */
        if (fgets(command, sizeof(command), stdin) == NULL) {
            if (g_shell_running) {  // Only print if not interrupted by signal
                printf("\nGoodbye!\n");
            }
            break; // EOF reached (Ctrl+D or piped input ended)
        }
        
        /* Check for buffer overflow */
        if (strlen(command) == sizeof(command) - 1 && command[sizeof(command) - 2] != '\n') {
            fprintf(stderr, "Error: Command too long (max %d characters)\n", COMMAND_BUFFER_SIZE - 1);
            
            // Clear the rest of the line
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        
        /* Remove newline character from input */
        command[strcspn(command, "\n")] = '\0';
        
        /* Skip empty commands */
        if (strlen(trim(command)) == 0) {
            continue;
        }
        
        /* Parse commands separated by pipes */
        command_count = 0;
        char *command_copy = safe_strdup(command);
        if (!command_copy) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            continue;
        }
        
        char *token = strtok(command_copy, "|");
        
        while (token != NULL && command_count < MAX_COMMANDS) {
            commands[command_count] = safe_strdup(token);
            if (!commands[command_count]) {
                // Cleanup on allocation failure
                cleanup_commands(commands, command_count);
                free(command_copy);
                fprintf(stderr, "Error: Memory allocation failed\n");
                goto continue_loop;
            }
            command_count++;
            token = strtok(NULL, "|");
        }
        free(command_copy);
        
        /* Check if we hit the command limit */
        if (token != NULL && command_count >= MAX_COMMANDS) {
            fprintf(stderr, "Warning: Too many piped commands (max %d), truncating\n", MAX_COMMANDS);
        }

        /* Debug output for testing purposes */
        if (getenv("SHELL_DEBUG")) {
            for (int i = 0; i < command_count; i++) {
                printf("Command %d: %s\n", i, trim(commands[i]));
                fflush(stdout);
            }
        }
        
        /* Execute commands */
        if (command_count == 1) {
            /* Single command execution */
            char* args[MAX_ARGS];
            char* cmd_copy = safe_strdup(commands[0]);
            if (cmd_copy) {
                trim(cmd_copy);
                
                if (parse_args(cmd_copy, args) > 0) {
                    execute_command(args);
                }
                free(cmd_copy);
            }
            
        } else if (command_count > 1) {
            /* Multiple commands with pipes */
            execute_pipe(commands, command_count);
        }
        
        /* Clean up allocated memory */
        cleanup_commands(commands, command_count);
        
        /* If input was from pipe, exit after processing one command */
        if (!isatty(STDIN_FILENO)) {
            break;
        }
        
        continue_loop:
        continue;
    }
    
    if (g_shell_running) {
        printf("\nShell exiting...\n");
    }
    return SUCCESS;
}
