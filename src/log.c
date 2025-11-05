// LLM GENERATED CODE BEGINS

#include"../include/header.h"

void handle_log(char *input, char *home_dir, char *pwd) {
    (void)home_dir; // Suppress unused parameter warning
    (void)pwd;      // Suppress unused parameter warning
    
    char *arg = input + 3;
    while (*arg == ' ')
    {
        arg++; // Skip spaces
    }

    if (*arg == '\0')
    {
        // No arguments: Print the log (oldest to newest)
        if (log_count == 0)
        {
            printf("No commands in log.\n");
        }
        else
        {
            int count = (log_count > MAX_LOG_SIZE) ? MAX_LOG_SIZE : log_count;
            int start = (log_count > MAX_LOG_SIZE) ? (log_count % MAX_LOG_SIZE) : 0;

            for (int i = 0; i < count; i++)
            {
                int index = (start + i) % MAX_LOG_SIZE;
                printf("%s\n", command_log[index]);
            }
        }
    }
    else if (strcmp(arg, "purge") == 0)
    {
        // Purge the log
        log_count = 0;
        printf("Command log cleared.\n");
    }
    else if (strncmp(arg, "execute ", 8) == 0)
    {
        // Execute a command from the log
        arg += 8;
        while (*arg == ' ')
        {
            arg++; // Skip spaces
        }

        if (*arg == '\0')
        {
            printf("log: Invalid Syntax!\n");
            return;
        }

        // Check if the argument is a valid number
        char *endptr;
        int index = strtol(arg, &endptr, 10);
        
        // Check if entire argument is a valid number
        if (*endptr != '\0' || index < 1)
        {
            printf("log: Invalid Syntax!\n");
            return;
        }

        int available_commands = (log_count > MAX_LOG_SIZE) ? MAX_LOG_SIZE : log_count;
        
        if (index > available_commands)
        {
            printf("log: Invalid index.\n");
            return;
        }

        // Calculate the actual index (newest to oldest indexing)
        // index 1 = newest, index 2 = second newest, etc.
        int actual_log_index;
        if (log_count > MAX_LOG_SIZE)
        {
            // When we have more than MAX_LOG_SIZE commands
            int newest_pos = (log_count - 1) % MAX_LOG_SIZE;
            actual_log_index = (newest_pos - index + 1 + MAX_LOG_SIZE) % MAX_LOG_SIZE;
        }
        else
        {
            // When we have less than MAX_LOG_SIZE commands
            actual_log_index = log_count - index;
        }

        // Get the command to execute
        char command[1024];
        strncpy(command, command_log[actual_log_index], sizeof(command) - 1);
        command[sizeof(command) - 1] = '\0';

        // CRITICAL FIX: Check if we're in a pipe context by examining if stdout is not a terminal
        // This detects when log execute is being piped to another command like wc -c
        if (!isatty(STDOUT_FILENO)) {
            // We're in a pipe, just output the command directly without "Executing:" prefix
            printf("%s", command);
        } else {
            // Normal case - show "Executing:" message and execute
            printf("Executing: %s\n", command);
            
            // Set a flag to prevent storing this executed command
            executing_from_log = 1;
            
            // Execute the command
            execute_sequential(command);
            
            executing_from_log = 0;
        }
    }
    else
    {
        printf("log: Invalid Syntax!\n");
    }
}

// Function to add command to log (call this from your main command processor)
void add_to_log(char *command)
{
    // Don't store if executing from log
    if (executing_from_log)
        return;
    
    // Don't store if command starts with "log"
    char *trimmed = command;
    while (*trimmed == ' ' || *trimmed == '\t')
        trimmed++;
    
    if (strncmp(trimmed, "log", 3) == 0)
    {
        // Check if it's exactly "log" or "log " followed by something
        char next_char = trimmed[3];
        if (next_char == '\0' || next_char == ' ' || next_char == '\t')
            return;
    }
    
    // Don't store if identical to previous command
    if (log_count > 0)
    {
        int prev_index = (log_count - 1) % MAX_LOG_SIZE;
        if (strcmp(command_log[prev_index], command) == 0)
            return;
    }
    
    // Store the command
    int store_index = log_count % MAX_LOG_SIZE;
    strncpy(command_log[store_index], command, sizeof(command_log[store_index]) - 1);
    command_log[store_index][sizeof(command_log[store_index]) - 1] = '\0';
    log_count++;
}

// Function to save log to file for persistence
void save_log_to_file(char *home_dir)
{
    char log_file_path[1024];
    snprintf(log_file_path, sizeof(log_file_path), "%s/.shell_log", home_dir);
    
    FILE *file = fopen(log_file_path, "w");
    if (!file) return;
    
    fprintf(file, "%d\n", log_count);
    
    if (log_count > 0)
    {
        int count = (log_count > MAX_LOG_SIZE) ? MAX_LOG_SIZE : log_count;
        int start = (log_count > MAX_LOG_SIZE) ? (log_count % MAX_LOG_SIZE) : 0;
        
        for (int i = 0; i < count; i++)
        {
            int index = (start + i) % MAX_LOG_SIZE;
            fprintf(file, "%s\n", command_log[index]);
        }
    }
    
    fclose(file);
}

// Function to load log from file for persistence
void load_log_from_file(char *home_dir)
{
    char log_file_path[1024];
    snprintf(log_file_path, sizeof(log_file_path), "%s/.shell_log", home_dir);
    
    FILE *file = fopen(log_file_path, "r");
    if (!file) 
    {
        log_count = 0;
        return;
    }
    
    if (fscanf(file, "%d", &log_count) != 1)
    {
        log_count = 0;
        fclose(file);
        return;
    }
    
    char line[1024];
    fgets(line, sizeof(line), file); // consume newline after count
    
    int commands_to_read = (log_count > MAX_LOG_SIZE) ? MAX_LOG_SIZE : log_count;
    
    for (int i = 0; i < commands_to_read; i++)
    {
        if (fgets(line, sizeof(line), file))
        {
            // Remove newline
            line[strcspn(line, "\n")] = '\0';
            
            int store_index = i % MAX_LOG_SIZE;
            strncpy(command_log[store_index], line, sizeof(command_log[store_index]) - 1);
            command_log[store_index][sizeof(command_log[store_index]) - 1] = '\0';
        }
    }
    
    fclose(file);
}

// LLM GENERATED CODE ENDS