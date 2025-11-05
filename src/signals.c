// LLM GENERATED CODE BEGINS
#include "../include/header.h"
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

void setup_signal_handlers() {
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
}

void sigint_handler(int sig) {
    (void)sig;
    
    if (current_fg_pid > 0) {
        kill(-current_fg_pid, SIGINT);
        current_fg_pid = 0;
    }
    
    write(STDOUT_FILENO, "\n", 1);
}

void sigtstp_handler(int sig) {
    (void)sig;
    
    if (current_fg_pid > 0) {
        // Send SIGTSTP to the foreground process group
        kill(-current_fg_pid, SIGTSTP);
        
        // Add job as stopped
        int job_id = add_job(current_fg_pid, fg_cmd, 1);
        
        // Print job info using signal-safe functions
        char msg[256];
        int len = snprintf(msg, sizeof(msg), "\n[%d] Stopped %s\n", job_id, fg_cmd);
        if (len > 0 && len < (int)sizeof(msg)) {
            write(STDOUT_FILENO, msg, (size_t)len);
        }
        
        // CRITICAL: Clear foreground process and give terminal control back to shell
        current_fg_pid = 0;
        
        // Give terminal control back to shell immediately
        tcsetpgrp(STDIN_FILENO, getpgrp());
    }
}

// LLM GENERATED CODE ENDS