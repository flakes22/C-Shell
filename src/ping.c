// LLM GENERATED CODE BEGINS
#include "../include/header.h"

void handle_ping(char **tokens) {
    if (!tokens[1] || !tokens[2] || tokens[3]) {
        printf("Invalid syntax!\n");
        return;
    }
    
    // Parse PID
    char *endptr;
    pid_t pid = strtol(tokens[1], &endptr, 10);
    if (*endptr != '\0') {
        printf("Invalid syntax!\n");
        return;
    }
    
    // Parse signal number
    int signal_number = strtol(tokens[2], &endptr, 10);
    if (*endptr != '\0') {
        printf("Invalid syntax!\n");
        return;
    }
    
    // Take signal number modulo 32
    int actual_signal = signal_number % 32;
    
    // Send signal to process
    if (kill(pid, actual_signal) == -1) {
        if (errno == ESRCH) {
            printf("No such process found\n");
        } else {
            perror("ping");
        }
        return;
    }
    
    printf("Sent signal %d to process with pid %d\n", signal_number, pid);
}

// LLM GENERATED CODE ENDS