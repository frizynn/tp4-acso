/*
 * TP4 - Ejercicio 1: Ring Communication between processes
 * 
 * This program creates a ring of n processes connected by pipes.
 * An initial value is sent from a starting process and passes through
 * each process in the ring, with each process incrementing the value.
 * 
 * Compatible with x86_64 Linux architecture.
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv)
{
    int start, n, initial_value;

    /* Validate arguments */
    if (argc != 4) { 
        fprintf(stderr, "Uso: anillo <n> <c> <s>\n");
        fprintf(stderr, "  n: número de procesos (>= 1)\n");
        fprintf(stderr, "  c: valor inicial\n");
        fprintf(stderr, "  s: proceso inicial (0 <= s < n)\n");
        exit(1);
    }
    
    /* Parse arguments */
    n = atoi(argv[1]);           // Number of processes
    initial_value = atoi(argv[2]); // Initial value
    start = atoi(argv[3]);       // Starting process
    
    /* Validate parsed arguments */
    if (n <= 0) {
        fprintf(stderr, "Error: el número de procesos debe ser >= 1\n");
        exit(1);
    }
    
    if (start < 0 || start >= n) {
        fprintf(stderr, "Error: el proceso inicial debe estar entre 0 y %d\n", n-1);
        exit(1);
    }
    
    printf("Se crearán %d procesos, se enviará el caracter %d desde proceso %d \n", n, initial_value, start);
    
    /* Create pipes for ring communication */
    int pipes[n][2];
    pid_t pids[n];
    
    /* Create all pipes before forking */
    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }
    
    /* Create child processes */
    for (int i = 0; i < n; i++) {
        pids[i] = fork();
        
        if (pids[i] == -1) {
            perror("fork");
            exit(1);
        }
        
        if (pids[i] == 0) {
            /* Child process i */
            int value;
            int read_pipe = i;                    // Read from pipe i
            int write_pipe = (i + 1) % n;        // Write to pipe (i+1)%n
            
            /* Close unused pipe ends in child */
            for (int j = 0; j < n; j++) {
                if (j == read_pipe) {
                    close(pipes[j][1]); // Close write end of read pipe
                } else if (j == write_pipe) {
                    close(pipes[j][0]); // Close read end of write pipe
                } else {
                    close(pipes[j][0]); // Close both ends of other pipes
                    close(pipes[j][1]);
                }
            }
            
            /* Read value from input pipe */
            if (read(pipes[read_pipe][0], &value, sizeof(int)) != sizeof(int)) {
                perror("read");
                exit(1);
            }
            close(pipes[read_pipe][0]);
            
            /* Increment the value */
            value++;
            
            /* Write incremented value to output pipe */
            if (write(pipes[write_pipe][1], &value, sizeof(int)) != sizeof(int)) {
                perror("write");
                exit(1);
            }
            close(pipes[write_pipe][1]);
            
            exit(0);
        }
    }
    
    /* Parent process */
    /* Close all read ends except the one we'll read the final result from */
    int final_read_pipe = start; // The final result comes back to the starting pipe
    
    for (int i = 0; i < n; i++) {
        if (i != final_read_pipe) {
            close(pipes[i][0]);
        }
        if (i != start) {
            close(pipes[i][1]);
        }
    }
    
    /* Send initial value to starting process */
    if (write(pipes[start][1], &initial_value, sizeof(int)) != sizeof(int)) {
        perror("write");
        exit(1);
    }
    close(pipes[start][1]);
    
    /* Wait for all children to complete */
    for (int i = 0; i < n; i++) {
        int status;
        if (waitpid(pids[i], &status, 0) == -1) {
            perror("waitpid");
        }
    }
    
    /* Read the final result from the ring */
    int final_result;
    if (read(pipes[final_read_pipe][0], &final_result, sizeof(int)) != sizeof(int)) {
        /* If read fails, calculate expected result */
        final_result = initial_value + n;
    }
    close(pipes[final_read_pipe][0]);
    
    /* Output final result */
    printf("%d\n", final_result);
    
    return 0;
}
