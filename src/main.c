#include "../include/header.h"

int tokenizer(char *input);
int valid_name(char *input);

int has_invalid_pipe_syntax(char *input) {
    char *pipe_pos = strchr(input, '|');
    if (!pipe_pos) return 0;
    
    char *after_pipe = pipe_pos + 1;
    while (*after_pipe == ' ' || *after_pipe == '\t') after_pipe++;
    if (*after_pipe == '\0' || *after_pipe == ';') {
        return 1; // Invalid if pipe is at end or followed by semicolon
    }
    
    char *before_pipe = pipe_pos - 1;
    while (before_pipe >= input && (*before_pipe == ' ' || *before_pipe == '\t')) before_pipe--;
    if (before_pipe < input) {
        return 1; 
    }
    
    return 0;
}

int main()
{
   
    init_globals();
    setpgid(0, 0);
    tcsetpgrp(STDIN_FILENO, getpgrp());
    
    setup_signal_handlers();
    
    int userid = getuid();
    struct passwd *pw = getpwuid(userid);

    char username[250];
    char sysname[250];
    if (pw && gethostname(sysname, sizeof(sysname)) == 0)
    {
        strcpy(username, pw->pw_name);
    }

    getcwd(home_dir, sizeof(home_dir));
    
    pwd[0] = '\0';  // Initialize pwd as empty string

    // Load log from file for persistence
    load_log_from_file(home_dir);

    while (1)
    {
        cleanup_jobs();

        getcwd(cwd, sizeof(cwd));
        
        // Replace home_dir with ~ if cwd starts with home_dir
        char display_path[1024];
        size_t home_len = strlen(home_dir);

        if (strncmp(cwd, home_dir, home_len) == 0 &&
            (cwd[home_len] == '/' || cwd[home_len] == '\0'))
        {
            // Replace home_dir with ~
            if (cwd[home_len] == '\0')
                snprintf(display_path, sizeof(display_path), "~"); // exactly home dir
            else
                snprintf(display_path, sizeof(display_path), "~%s", cwd + home_len);
        }
        else
        {
            snprintf(display_path, sizeof(display_path), "%s", cwd);
        }
        tcsetpgrp(STDIN_FILENO, getpgrp());
        
        printf("<%s@%s:%s> ", pw->pw_name, sysname, display_path);
        fflush(stdout);

        // INPUT
        char input[256];
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            // EOF detected-Ctrl+D
            printf("logout\n");
            
            for (int i = 0; i < MAX_JOBS; i++) {
                if (jobs[i].job_id != -1) {
                    kill(-jobs[i].pgid, SIGKILL);
                }
            }
            for (int i = 0; i < job_count; i++) {
                if (bg_pids[i] != -1) {
                    kill(-bg_pids[i], SIGKILL);
                }
            }
            
            // Save log before exiting
            save_log_to_file(home_dir);
            exit(0);
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0)
        {
            continue;
        }

        if (has_invalid_pipe_syntax(input)) {
            printf("Invalid Syntax!\n");
            continue;
        }

        add_to_log(input);

        if (strchr(input, ';'))
        {
            execute_sequential(input);
        }
        else 
        {
            int background = 0;
            int len = strlen(input);

            if (len > 0) {
                int last_char_pos = len - 1;
                while (last_char_pos >= 0 && (input[last_char_pos] == ' ' || input[last_char_pos] == '\t')) {
                    last_char_pos--;
                }
                
                if (last_char_pos >= 0 && input[last_char_pos] == '&') {
                    if (last_char_pos == 0 || input[last_char_pos-1] == ' ' || input[last_char_pos-1] == '\t') {
                        background = 1;
                        input[last_char_pos] = '\0';
                    
                        len = last_char_pos;
                        while (len > 0 && (input[len-1] == ' ' || input[len-1] == '\t')) {
                            input[--len] = '\0';
                        }
                    }
                }
            }
   
            if (strchr(input, '<') || strchr(input, '>') || strchr(input, '|')) {
                execute_command(input, background);
            }
            else if (strncmp(input, "hop", 3) == 0) {
                handle_hop(input, cwd, home_dir, pwd);
            }
            else if (strncmp(input, "reveal", 6) == 0) {
                handle_reveal(input, home_dir, pwd);
            }
            else if (strncmp(input, "log", 3) == 0) {
                handle_log(input, home_dir, pwd);
            }
            else if (strcmp(input, "activities") == 0) {
                handle_activities();
            }
            else if (strncmp(input, "ping", 4) == 0) {
                tokenizer(input);
                handle_ping(tokens);
            }
            else if (strncmp(input, "fg", 2) == 0) {
                handle_fg(input);
            }
            else if (strncmp(input, "bg", 2) == 0) {
                handle_bg(input);
            }
            else {
                execute_command(input, background);
            }
        }
    }

    save_log_to_file(home_dir);
    return 0;
}