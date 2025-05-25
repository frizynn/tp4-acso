#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main() {
    system("cd ../../src/ej2 && make clean && make");
    
    FILE* fp = popen("cd ../../src/ej2 && printf 'ls | grep test\\n' | SHELL_DEBUG=1 ./shell 2>&1 | head -10", "r");
    char output[1024] = {0};
    char line[256];
    int lines = 0;
    
    while (fgets(line, sizeof(line), fp) && lines < 10) {
        strcat(output, line);
        lines++;
    }
    pclose(fp);
    
    printf("DEBUG: Output received:\n'%s'\n", output);
    printf("DEBUG: Looking for 'Command 0': %s\n", strstr(output, "Command 0") ? "FOUND" : "NOT FOUND");
    printf("DEBUG: Looking for 'Command 1': %s\n", strstr(output, "Command 1") ? "FOUND" : "NOT FOUND");
    
    return 0;
}
