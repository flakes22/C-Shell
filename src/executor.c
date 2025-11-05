#include "../include/header.h"

int count_redirections(char **args, int argc) {
    int input_count = 0;
    int output_count = 0;
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(args[i], "<") == 0) {
            input_count++;
        }
        else if (strcmp(args[i], ">") == 0) {
            output_count++;
        }
    }
    
    // Return -1 if multiple redirects of same type found
    if (input_count > 1 || output_count > 1) {
        return -1;
    }
    
    return 0;
}

typedef struct {
    char *input_file;
    char *output_file;
    int output_mode; // 0 = no redirect, 1 = >, 2 = >>
} redirection_t;

redirection_t parse_redirections(char **args, int *argc) {
    redirection_t redir = {NULL, NULL, 0};
    int new_argc = 0;
    
    // Parse and validate all redirection tokens (bash behavior)
    for (int i = 0; i < *argc; i++) {
        if (strcmp(args[i], "<") == 0) {
            // Input redirection
            if (i + 1 < *argc) {
                // Check if file exists (even if we won't use it)
                if (access(args[i + 1], R_OK) != 0) {
                    printf("No such file or directory\n");
                    redir.input_file = NULL;
                    redir.output_file = NULL;
                    return redir;
                }
                // Keep the last input redirection
                redir.input_file = args[i + 1];
                i++; // Skip the filename
            }
        }
        else if (strcmp(args[i], ">") == 0) {
            // Output redirection (overwrite)
            if (i + 1 < *argc) {
                // Try to create/open the file (even if we won't use it)
                int test_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (test_fd < 0) {
                    printf("Unable to create file for writing\n");
                    redir.input_file = NULL;
                    redir.output_file = NULL;
                    return redir;
                }
                close(test_fd);
                
                // Keep the last output redirection
                redir.output_file = args[i + 1];
                redir.output_mode = 1;
                i++; // Skip the filename
            }
        }
        else if (strcmp(args[i], ">>") == 0) {
            // Output redirection (append)
            if (i + 1 < *argc) {
                // Try to create/open the file (even if we won't use it)
                int test_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                if (test_fd < 0) {
                    printf("Unable to create file for writing\n");
                    redir.input_file = NULL;
                    redir.output_file = NULL;
                    return redir;
                }
                close(test_fd);
                
                // Keep the last output redirection
                redir.output_file = args[i + 1];
                redir.output_mode = 2;
                i++; // Skip the filename
            }
        }
        else {
            // Regular argument - keep it
            args[new_argc++] = args[i];
        }
    }
    
    args[new_argc] = NULL;
    *argc = new_argc;
    return redir;
}

void execute_pipe_command(char *input, int background) {
    char *commands[10];
    int cmd_count = 0;
    
    // Split by pipe
    char *input_copy = malloc(strlen(input) + 1);
    strcpy(input_copy, input);
    
    char *token = strtok(input_copy, "|");
    while (token != NULL && cmd_count < 9) {
        // Trim spaces
        while (*token == ' ') token++;
        int len = strlen(token);
        while (len > 0 && token[len-1] == ' ') token[--len] = '\0';
        
        commands[cmd_count++] = token;
        token = strtok(NULL, "|");
    }
    
    if (cmd_count < 2) {
        free(input_copy);
        return;
    }
    
    int pipes[cmd_count-1][2];
    pid_t pids[cmd_count];
    
    // Create pipes
    for (int i = 0; i < cmd_count - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe failed");
            free(input_copy);
            return;
        }
    }
    
    // Execute commands
    for (int i = 0; i < cmd_count; i++) {
        pids[i] = fork();
        
        if (pids[i] == 0) {
            // Child process
            
            // Create new process group for job control
            if (setpgid(0, 0) == -1) {
                perror("setpgid failed");
                exit(1);
            }
            
            // Reset signal handlers to default in child
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            
            // Setup input from previous pipe (if not first command)
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            // Setup output to next pipe (if not last command)
            if (i < cmd_count - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            // Close all pipe fds
            for (int j = 0; j < cmd_count - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Parse command arguments and handle redirections
            char *args[100];
            int argc = 0;
            char *cmd_copy = malloc(strlen(commands[i]) + 1);
            strcpy(cmd_copy, commands[i]);
            
            // Parse arguments using strtok
            char *arg_token = strtok(cmd_copy, " \t\n");
            while (arg_token && argc < 99) {
                args[argc++] = arg_token;
                arg_token = strtok(NULL, " \t\n");
            }
            args[argc] = NULL;
            
            if (argc == 0) {
                free(cmd_copy);
                exit(1);
            }
            
            // Parse redirections for this command
            redirection_t redir = parse_redirections(args, &argc);
            
            // Handle input redirection (only for first command or if explicitly specified)
            if (redir.input_file && i == 0) {
                int input_fd = open(redir.input_file, O_RDONLY);
                if (input_fd < 0) {
                    printf("No such file or directory\n");
                    exit(1);
                }
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }
            
            // Handle output redirection (only for last command or if explicitly specified)
            if (redir.output_file && i == cmd_count - 1) {
                int output_fd;
                if (redir.output_mode == 1) {
                    // Overwrite mode
                    output_fd = open(redir.output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                } else if (redir.output_mode == 2) {
                    // Append mode
                    output_fd = open(redir.output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                } else {
                    output_fd = -1;
                }
                
                if (output_fd < 0) {
                    printf("Unable to create file for writing\n");
                    exit(1);
                }
                dup2(output_fd, STDOUT_FILENO);
                close(output_fd);
            }
            
            // Check if this is a built-in command
            if (argc > 0) {
                // Reconstruct command string for built-in handlers
                char reconstructed[1024] = "";
                for (int k = 0; k < argc; k++) {
                    if (k > 0) strcat(reconstructed, " ");
                    strcat(reconstructed, args[k]);
                }
                
                if (strncmp(args[0], "reveal", 6) == 0) {
                    handle_reveal(reconstructed, home_dir, pwd);
                    free(cmd_copy);
                    exit(0);
                }
                else if (strncmp(args[0], "hop", 3) == 0) {
                    handle_hop(reconstructed, cwd, home_dir, pwd);
                    free(cmd_copy);
                    exit(0);
                }
                else if (strncmp(args[0], "log", 3) == 0) {
                    handle_log(reconstructed, home_dir, pwd);
                    free(cmd_copy);
                    exit(0);
                }
                else if (strcmp(args[0], "activities") == 0) {
                    handle_activities();
                    free(cmd_copy);
                    exit(0);
                }
            }
            
            // Execute external command
            execvp(args[0], args);
            printf("Command not found!\n");
            free(cmd_copy);
            exit(1);
        }
        else if (pids[i] > 0) {
            // Parent - set process group for job control
            setpgid(pids[i], pids[0]); // All processes in same group
        }
    }
    
    // Close all pipes in parent
    for (int i = 0; i < cmd_count - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Handle job control for the pipeline
    if (background) {
        // Add to background jobs (use the process group leader)
        if (job_count < 100) {
            bg_pids[job_count] = pids[0];
            bg_cmds[job_count] = malloc(strlen(input) + 1);
            strcpy(bg_cmds[job_count], input);
            
            fprintf(stderr, "[%d] %d\n", job_count + 1, pids[0]);
            job_count++;
        }
        add_job(pids[0], input, 0);
    } else {
        // Foreground pipeline
        current_fg_pid = pids[0];
        strncpy(fg_cmd, input, sizeof(fg_cmd) - 1);
        fg_cmd[sizeof(fg_cmd) - 1] = '\0';
        
        // Give terminal control to the pipeline
        tcsetpgrp(STDIN_FILENO, pids[0]);
        
        // Wait for all children
        for (int i = 0; i < cmd_count; i++) {
            int status;
            pid_t result = waitpid(pids[i], &status, WUNTRACED);
            
            if (result > 0 && WIFSTOPPED(status)) {
                // One of the processes was stopped
                int job_id = add_job(pids[0], input, 1);
                fprintf(stderr, "[%d] Stopped %s\n", job_id, input);
                break;
            }
        }
        
        // Restore terminal control to shell
        tcsetpgrp(STDIN_FILENO, getpgrp());
        current_fg_pid = 0;
    }
    
    free(input_copy);
}

void execute_command(char *input, int background)
{
    // Check for pipes first
    if (strchr(input, '|')) {
        execute_pipe_command(input, background);
        return;
    }
    
    char *args[256];
    int argc = 0;
    
    // Parse command into arguments
    char *input_copy = malloc(strlen(input) + 1);
    strcpy(input_copy, input);
    
    char *token = strtok(input_copy, " \t\n");
    while (token != NULL && argc < 255)
    {
        args[argc++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[argc] = NULL;
    
    if (argc == 0) {
        free(input_copy);
        return;
    }
    
    // Parse redirections
    redirection_t redir = parse_redirections(args, &argc);
    
    pid_t pid = fork();
    
    if (pid == 0)
    {
        // Child process
        
        // Create new process group - this is crucial for job control
        if (setpgid(0, 0) == -1) {
            perror("setpgid failed");
            exit(1);
        }
        
        // For foreground processes, give them terminal control
        if (!background) {
            if (tcsetpgrp(STDIN_FILENO, getpid()) == -1) {
                // Don't exit on this error, just continue
                // perror("tcsetpgrp failed");
            }
        }
        
        // Reset signal handlers to default in child
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        
        // Handle input redirection
        if (redir.input_file) {
            int input_fd = open(redir.input_file, O_RDONLY);
            if (input_fd < 0) {
                printf("No such file or directory\n");
                exit(1);
            }
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        else if (background) {
            // For background processes, redirect stdin from /dev/null if no input redirection
            int null_fd = open("/dev/null", O_RDONLY);
            if (null_fd != -1) {
                dup2(null_fd, STDIN_FILENO);
                close(null_fd);
            }
        }
        
        // Handle output redirection
        if (redir.output_file) {
            int output_fd;
            if (redir.output_mode == 1) {
                // Overwrite mode
                output_fd = open(redir.output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            } else if (redir.output_mode == 2) {
                // Append mode
                output_fd = open(redir.output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            }
            
            if (output_fd < 0) {
                printf("Unable to create file for writing\n");
                exit(1);
            }
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }
        
        // Check if this is a built-in command
        if (argc > 0) {
            // Reconstruct command string for built-in handlers
            char reconstructed[1024] = "";
            for (int k = 0; k < argc; k++) {
                if (k > 0) strcat(reconstructed, " ");
                strcat(reconstructed, args[k]);
            }
            
            if (strncmp(args[0], "reveal", 6) == 0) {
                handle_reveal(reconstructed, home_dir, pwd);
                free(input_copy);
                exit(0);
            }
            else if (strncmp(args[0], "hop", 3) == 0) {
                handle_hop(reconstructed, cwd, home_dir, pwd);
                free(input_copy);
                exit(0);
            }
            else if (strncmp(args[0], "log", 3) == 0) {
                handle_log(reconstructed, home_dir, pwd);
                free(input_copy);
                exit(0);
            }
            else if (strcmp(args[0], "activities") == 0) {
                handle_activities();
                free(input_copy);
                exit(0);
            }
        }
        
        // Execute external command
        execvp(args[0], args);
        
        // If execvp returns, there was an error
        printf("Command not found!\n");
        exit(1);
    }
    else if (pid > 0)
    {
        // Parent process
        
        // Set the child's process group
        setpgid(pid, pid);
        
        if (background)
        {
            // Add to background jobs
            if (job_count < 100)
            {
                bg_pids[job_count] = pid;
                bg_cmds[job_count] = malloc(strlen(input) + 1);
                strcpy(bg_cmds[job_count], input);
                
                // Print job info
                fprintf(stderr,"[%d] %d\n", job_count + 1, pid);
                job_count++;
            }
            
            // Also add to new job system
            add_job(pid, input, 0);
        }
        else
        {
            // Foreground process
            current_fg_pid = pid;
            strncpy(fg_cmd, input, sizeof(fg_cmd) - 1);
            fg_cmd[sizeof(fg_cmd) - 1] = '\0';
            
            // Give terminal control to the child process group
            tcsetpgrp(STDIN_FILENO, pid);
            
            // Wait for completion
            int status;
            pid_t result = waitpid(pid, &status, WUNTRACED);
            
            if (result > 0) {
                if (WIFSTOPPED(status)) {
                    // Process was stopped, add to jobs
                    int job_id = add_job(pid, input, 1);
                    fprintf(stderr, "[%d] Stopped %s\n", job_id, input);
                } else {
                    // Process completed normally
                    current_fg_pid = 0;
                }
            }
            
            // Restore terminal control to shell
            tcsetpgrp(STDIN_FILENO, getpgrp());
            current_fg_pid = 0;
        }
    }
    else
    {
        perror("fork failed");
    }
    
    free(input_copy);
}