/*
 * TP4 - Ejercicio 2: Shell Implementation with Pipe Support
 * 
 * This shell implementation provides:
 * - Interactive command prompt
 * - Single command execution
 * - Pipe command chaining (command1 | command2 | ...)
 * - Built-in commands (exit)
 * - Proper process management and cleanup
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#define MAX_COMMANDS 200
#define MAX_ARGS 64

/* Function to trim whitespace from both ends of a string */
char* trim(char* str) {
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

/* Function to parse command line arguments */
int parse_args(char* command, char** args) {
    int argc = 0;
    char* token = strtok(command, " \t\n");
    
    while (token != NULL && argc < MAX_ARGS - 1) {
        args[argc++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[argc] = NULL;
    return argc;
}

/* Function to execute a single command */
int execute_command(char** args) {
    if (args[0] == NULL) {
        return 1; // Empty command
    }
    
    // Handle built-in commands
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("execvp");
            exit(1);
        }
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    } else {
        perror("fork");
        return 1;
    }
    return 0;
}

/* Function to execute commands connected by pipes */
int execute_pipe(char** commands, int num_commands) {
    int pipes[num_commands - 1][2];
    pid_t pids[num_commands];
    
    // Create all pipes
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return 1;
        }
    }
    
    // Create child processes for each command
    for (int i = 0; i < num_commands; i++) {
        char* args[MAX_ARGS];
        char* cmd_copy = strdup(commands[i]);
        trim(cmd_copy);
        
        if (parse_args(cmd_copy, args) == 0) {
            free(cmd_copy);
            continue; // Skip empty commands
        }
        
        pids[i] = fork();
        
        if (pids[i] == 0) {
            // Child process
            
            // Set up input redirection
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            // Set up output redirection
            if (i < num_commands - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            // Close all pipe file descriptors
            for (int j = 0; j < num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Execute the command
            if (execvp(args[0], args) == -1) {
                perror("execvp");
                exit(1);
            }
        } else if (pids[i] == -1) {
            perror("fork");
            free(cmd_copy);
            return 1;
        }
        
        free(cmd_copy);
    }
    
    // Close all pipe file descriptors in parent
    for (int i = 0; i < num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Wait for all child processes
    for (int i = 0; i < num_commands; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }
    
    return 0;
}

int main() {
    char command[256];
    char *commands[MAX_COMMANDS];
    int command_count;

    printf("Simple Shell with Pipe Support\n");
    printf("Type 'exit' to quit\n\n");

    /* Main shell loop */
    while (1) {
        /* Display prompt */
        printf("Shell> ");
        fflush(stdout); // Ensure prompt is displayed immediately
        
        /* Read user input */
        if (fgets(command, sizeof(command), stdin) == NULL) {
            printf("\nGoodbye!\n");
            break; // EOF reached (Ctrl+D or piped input ended)
        }
        
        /* Remove newline character from input */
        command[strcspn(command, "\n")] = '\0';
        
        /* Skip empty commands */
        if (strlen(trim(command)) == 0) {
            continue;
        }
        
        /* Parse commands separated by pipes */
        command_count = 0;
        char *command_copy = strdup(command);
        char *token = strtok(command_copy, "|");
        
        while (token != NULL && command_count < MAX_COMMANDS) {
            commands[command_count++] = strdup(token);
            token = strtok(NULL, "|");
        }
        free(command_copy);

        // Commands parsed successfully
        
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
            char* cmd_copy = strdup(commands[0]);
            trim(cmd_copy);
            
            if (parse_args(cmd_copy, args) > 0) {
                execute_command(args);
            }
            free(cmd_copy);
            
        } else if (command_count > 1) {
            /* Multiple commands with pipes */
            execute_pipe(commands, command_count);
        }
        
        /* Clean up allocated memory */
        for (int i = 0; i < command_count; i++) {
            free(commands[i]);
        }
        
        /* If input was from pipe, exit after processing one command */
        if (!isatty(STDIN_FILENO)) {
            break;
        }
    }
    return 0;
}
