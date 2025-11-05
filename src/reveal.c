// LLM GENERATED CODE BEGINS
#include "../include/header.h"

void handle_reveal(char *input, char *home_dir, char *pwd) {
    (void)home_dir; // Suppress unused parameter warning
    
    if (!input) {
        printf("Invalid input\n");
        return;
    }
    
    char *input_copy = malloc(strlen(input) + 1);
    strcpy(input_copy, input);
    
    char *args[100];
    int argc = 0;
    int show_all = 0;
    int long_format = 0;
    char *target_dirs[10];
    int dir_count = 0;
    
    // Parse arguments
    char *token = strtok(input_copy, " \t");
    if (token != NULL) {
        token = strtok(NULL, " \t"); // Skip "reveal"
    }
    
    while (token != NULL && argc < 99) {
        if (token[0] == '-' && strlen(token) > 1 && strcmp(token, "-") != 0) {
            // Parse flags (but not "-" which means previous directory)
            for (int i = 1; token[i] != '\0'; i++) {
                if (token[i] == 'a') show_all = 1;
                if (token[i] == 'l') long_format = 1;
            }
        } else {
            // Directory argument (including "-" for previous directory)
            if (dir_count < 10) {
                target_dirs[dir_count++] = token;
            } else {
                printf("reveal: Invalid Syntax!\n");
                free(input_copy);
                return;
            }
        }
        token = strtok(NULL, " \t");
        argc++;
    }
    
    // Check for too many arguments
    if (dir_count > 3) {
        printf("reveal: Invalid Syntax!\n");
        free(input_copy);
        return;
    }
    
    // If no directory specified, use current directory
    if (dir_count == 0) {
        target_dirs[0] = ".";
        dir_count = 1;
    }
    
    // Process each directory
    for (int d = 0; d < dir_count; d++) {
        char *target_dir = target_dirs[d];
        
        // CRITICAL FIX: Handle special case for "-" (previous directory)
        if (strcmp(target_dir, "-") == 0) {
            // Check if pwd is unset or empty
            if (pwd == NULL || strlen(pwd) == 0) {
                printf("No such directory!\n");
                continue;
            }
            target_dir = pwd;
        }
        
        // Check if directory exists and is accessible
        struct stat st;
        if (stat(target_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
            printf("No such directory!\n");
            continue;
        }
        
        // Check if directory exists
        DIR *dir = opendir(target_dir);
        if (!dir) {
            printf("No such directory!\n");
            continue;
        }
        
        // Print directory header if multiple directories
        if (dir_count > 1) {
            if (d > 0) printf("\n"); // Add newline between directories
            if (strcmp(target_dirs[d], "-") == 0) {
                printf("%s:\n", pwd);
            } else {
                printf("%s:\n", target_dirs[d]);
            }
        }
        
        // Read directory contents
        struct dirent *entry;
        char *files[1000];
        int file_count = 0;
        
        while ((entry = readdir(dir)) != NULL && file_count < 1000) {
            if (!show_all && entry->d_name[0] == '.') {
                // Skip hidden files unless -a flag is used
                continue;
            }
            
            files[file_count] = malloc(strlen(entry->d_name) + 1);
            strcpy(files[file_count], entry->d_name);
            file_count++;
        }
        closedir(dir);
        
        // Sort files alphabetically
        for (int i = 0; i < file_count - 1; i++) {
            for (int j = i + 1; j < file_count; j++) {
                if (strcmp(files[i], files[j]) > 0) {
                    char *temp = files[i];
                    files[i] = files[j];
                    files[j] = temp;
                }
            }
        }
        
        // Print files
        for (int i = 0; i < file_count; i++) {
            printf("%s\n", files[i]);
            free(files[i]);
        }
    }
    
    free(input_copy);
}
// LLM GENERATED CODE ENDS