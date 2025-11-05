#include "../include/header.h"

void handle_hop(char *input, char *cwd, char *home_dir, char *pwd) {
   
    char *args = input + 3;
    while (*args == ' ') args++;
    
    if (*args == '\0') { // hop
        chdir(home_dir);
        return;
    }
    
    char args_copy[1024];
    strncpy(args_copy, args, sizeof(args_copy) - 1);
    args_copy[sizeof(args_copy) - 1] = '\0';
    
    char original_dir[1024];
    getcwd(original_dir, sizeof(original_dir));
    
    char *token = strtok(args_copy, " \t");
    while (token != NULL) {
        if (strcmp(token, "~") == 0) {
            if (chdir(home_dir) != 0) {
                printf("No such directory!\n");
                chdir(original_dir);
                return;
            }
        }
        else if (strcmp(token, "-") == 0) {
            if (strlen(pwd) > 0) {
                char temp[1024];
                getcwd(temp, sizeof(temp));
                if (chdir(pwd) != 0) {
                    printf("No such directory!\n");
                    chdir(original_dir);
                    return;
                }
                strcpy(pwd, temp);
            }
        }
        else if (token[0] == '/') {
            if (chdir("/") != 0) {
                printf("No such directory!\n");
                chdir(original_dir);
                return;
            }
            
            if (strlen(token) > 1) {
                if (chdir(token + 1) != 0) {
                    printf("No such directory!\n");
                    chdir(original_dir);
                    return;
                }
            }
            
            strcpy(pwd, original_dir);
        }
        else {
            // Relative path
            if (chdir(token) != 0) {
                printf("No such directory!\n");
                chdir(original_dir);
                return;
            }
            
            strcpy(pwd, original_dir);
        }
        
        token = strtok(NULL, " \t");
    }
    
   
    if (strcmp(original_dir, getcwd(NULL, 0)) != 0) {
        strcpy(pwd, original_dir);
    }
}
