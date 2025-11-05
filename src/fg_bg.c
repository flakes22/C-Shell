// LLM GENERATED CODE BEGINS
#include "../include/header.h"

void handle_fg(char *input) {
    char *args[10];
    int argc = 0;
    
    // Parse arguments
    char *input_copy = malloc(strlen(input) + 1);
    strcpy(input_copy, input);
    char *token = strtok(input_copy, " \t");
    while (token != NULL && argc < 9) {
        args[argc++] = token;
        token = strtok(NULL, " \t");
    }
    
    Job *job = NULL;
    
    if (argc == 1) {
        // No job number provided, use most recent
        job = get_most_recent_job();
        if (!job) {
            printf("No such job\n");
            free(input_copy);
            return;
        }
    } else if (argc == 2) {
        // Job number provided
        char *endptr;
        int job_id = strtol(args[1], &endptr, 10);
        if (*endptr != '\0') {
            printf("No such job\n");
            free(input_copy);
            return;
        }
        job = find_job_by_id(job_id);
        if (!job) {
            printf("No such job\n");
            free(input_copy);
            return;
        }
    } else {
        printf("fg: Invalid syntax\n");
        free(input_copy);
        return;
    }
    
    // Print the command being brought to foreground
    printf("%s\n", job->command);
    
    // If job is stopped, send SIGCONT to resume it
    if (job->status == 1) {
        kill(-job->pgid, SIGCONT);
        job->status = 0; // Mark as running
    }
    
    // Set as current foreground process
    current_fg_pid = job->pgid;
    strncpy(fg_cmd, job->command, sizeof(fg_cmd) - 1);
    fg_cmd[sizeof(fg_cmd) - 1] = '\0';
    
    // Wait for the job to complete or stop
    int status;
    pid_t result = waitpid(job->pgid, &status, WUNTRACED);
    
    if (result > 0) {
        if (WIFSTOPPED(status)) {
            // Process was stopped, keep it in jobs list
            job->status = 1;
            printf("[%d] Stopped %s\n", job->job_id, job->command);
        } else {
            // Process completed, remove from jobs list
            remove_job(job->job_id);
        }
    }
    
    current_fg_pid = 0; // No longer in foreground
    free(input_copy);
}

void handle_bg(char *input) {
    char *args[10];
    int argc = 0;
    
    // Parse arguments
    char *input_copy = malloc(strlen(input) + 1);
    strcpy(input_copy, input);
    char *token = strtok(input_copy, " \t");
    while (token != NULL && argc < 9) {
        args[argc++] = token;
        token = strtok(NULL, " \t");
    }
    
    Job *job = NULL;
    
    if (argc == 1) {
        // No job number provided, use most recent
        job = get_most_recent_job();
        if (!job) {
            printf("No such job\n");
            free(input_copy);
            return;
        }
    } else if (argc == 2) {
        // Job number provided
        char *endptr;
        int job_id = strtol(args[1], &endptr, 10);
        if (*endptr != '\0') {
            printf("No such job\n");
            free(input_copy);
            return;
        }
        job = find_job_by_id(job_id);
        if (!job) {
            printf("No such job\n");
            free(input_copy);
            return;
        }
    } else {
        printf("bg: Invalid syntax\n");
        free(input_copy);
        return;
    }
    
    if (job->status == 0) {
        // Job is already running
        printf("Job already running\n");
        free(input_copy);
        return;
    }
    
    // Resume the stopped job
    kill(-job->pgid, SIGCONT);
    job->status = 0; // Mark as running
    
    printf("[%d] %s &\n", job->job_id, job->command);
    free(input_copy);
}

// LLM GENERATED CODE ENDS