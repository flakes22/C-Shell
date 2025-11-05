// LLM GENERATED CODE ENDS
#include "../include/header.h"

// Function to check if a command contains redirection operators
int has_redirection(char *cmd) {
    return (strstr(cmd, "<") != NULL || strstr(cmd, ">") != NULL);
}

void execute_sequential(char *input) {
    char *saveptr_seq;
    char *input_copy = malloc(strlen(input) + 1);
    strcpy(input_copy, input);
    
    char *seq_cmd = strtok_r(input_copy, ";", &saveptr_seq);

    while (seq_cmd != NULL) {
        // Remove leading spaces
        while (*seq_cmd == ' ' || *seq_cmd == '\t') {
            seq_cmd++;
        }
        
        if (*seq_cmd == '\0') {
            seq_cmd = strtok_r(NULL, ";", &saveptr_seq);
            continue;
        }
        
        // Remove trailing spaces
        int len = strlen(seq_cmd);
        while (len > 0 && (seq_cmd[len-1] == ' ' || seq_cmd[len-1] == '\t')) {
            seq_cmd[--len] = '\0';
        }
        
        if (len == 0) {
            seq_cmd = strtok_r(NULL, ";", &saveptr_seq);
            continue;
        }
        
        // Make a copy of the command for parsing and processing
        char cmd_copy[1024];
        strncpy(cmd_copy, seq_cmd, sizeof(cmd_copy) - 1);
        cmd_copy[sizeof(cmd_copy) - 1] = '\0';
        
        // CRITICAL FIX: Parse the command to find & and split it correctly
        char *bg_cmd = NULL;
        int background = 0;
        
        // Find the first '&' that's not within quotes or escaped
        char *bg_pos = NULL;
        int in_quotes = 0;
        for (char *p = cmd_copy; *p != '\0'; p++) {
            if (*p == '"' || *p == '\'') {
                in_quotes = !in_quotes;
            } else if (*p == '&' && !in_quotes && (p == cmd_copy || *(p-1) != '\\')) {
                bg_pos = p;
                break;
            }
        }
        
        // If we found a proper & for background
        if (bg_pos != NULL) {
            // Check if & is followed by another command or is just a background marker
            char *next_part = bg_pos + 1;
            while (*next_part == ' ' || *next_part == '\t') next_part++;
            
            if (*next_part != '\0') {
                // & is followed by another command, this is the form "cmd1 & cmd2"
                bg_cmd = next_part;
                *bg_pos = '\0'; // Terminate the first command at &
            } else {
                // & is just a background marker for the command, form "cmd &"
                background = 1;
                *bg_pos = '\0'; // Remove the & from the command
            }
            
            // Trim trailing spaces after removing &
            len = strlen(cmd_copy);
            while (len > 0 && (cmd_copy[len-1] == ' ' || cmd_copy[len-1] == '\t')) {
                cmd_copy[--len] = '\0';
            }
        }
        
        // Handle the current command
        if (strchr(cmd_copy, '<') || strchr(cmd_copy, '>') || strchr(cmd_copy, '|')) {
            execute_command(cmd_copy, background);
        }
        else if (strncmp(cmd_copy, "hop", 3) == 0) {
            handle_hop(cmd_copy, cwd, home_dir, pwd);
        }
        else if (strncmp(cmd_copy, "reveal", 6) == 0) {
            handle_reveal(cmd_copy, home_dir, pwd);
        }
        else if (strncmp(cmd_copy, "log", 3) == 0) {
            handle_log(cmd_copy, home_dir, pwd);
        }
        else if (strcmp(cmd_copy, "activities") == 0) {
            handle_activities();
        }
        else if (strncmp(cmd_copy, "ping", 4) == 0) {
            // Tokenize for ping command
            char *ping_tokens[100];
            int ping_argc = 0;
            char *tok_save;
            char ping_copy[1024];
            strcpy(ping_copy, cmd_copy);
            char *arg = strtok_r(ping_copy, " \t", &tok_save);
            while (arg != NULL && ping_argc < 99) {
                ping_tokens[ping_argc++] = arg;
                arg = strtok_r(NULL, " \t", &tok_save);
            }
            ping_tokens[ping_argc] = NULL;
            handle_ping(ping_tokens);
        }
        else if (strncmp(cmd_copy, "fg", 2) == 0) {
            handle_fg(cmd_copy);
        }
        else if (strncmp(cmd_copy, "bg", 2) == 0) {
            handle_bg(cmd_copy);
        }
        else {
            // Use execute_command for external commands
            execute_command(cmd_copy, background);
        }
        
        // If there was a command after &, process it now
        if (bg_cmd != NULL) {
            // Create a new temporary string with the remaining commands
            char *remaining = malloc(strlen(bg_cmd) + strlen(seq_cmd + strlen(cmd_copy) + 1) + 2);
            strcpy(remaining, bg_cmd);
            
            // If there are more commands in the original sequence, append them
            char *next_seq = strtok_r(NULL, ";", &saveptr_seq);
            if (next_seq != NULL) {
                strcat(remaining, ";");
                strcat(remaining, next_seq);
            }
            
            // Recursively process the remaining commands
            execute_sequential(remaining);
            free(remaining);
            break; // Exit the current loop as we've handled all commands
        }
        
        // Move to the next sequential command
        seq_cmd = strtok_r(NULL, ";", &saveptr_seq);
    }
    
    free(input_copy);
}

// LLM GENERATED CODE ENDS