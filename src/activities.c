// LLM GENERATED CODE BEGINS
#include "../include/header.h"

void handle_activities()
{
    cleanup_jobs();

    // Collect active jobs from the new jobs array
    Job active_jobs[MAX_JOBS];
    int active_count = 0;
    
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].job_id != -1)
        {
            active_jobs[active_count++] = jobs[i];
        }
    }
    
    if (active_count == 0)
    {
        printf("No active background processes.\n");
        return;
    }
    
    // Sort by command name (lexicographically)
    for (int i = 0; i < active_count - 1; i++) {
        for (int j = i + 1; j < active_count; j++) {
            if (strcmp(active_jobs[i].command, active_jobs[j].command) > 0) {
                Job temp = active_jobs[i];
                active_jobs[i] = active_jobs[j];
                active_jobs[j] = temp;
            }
        }
    }
    
    for (int i = 0; i < active_count; i++)
    {
        char *state = (active_jobs[i].status == 0) ? "Running" : "Stopped";
        printf("[%d] : %s - %s\n", active_jobs[i].pgid, active_jobs[i].command, state);
    }
}

// Function to check and clean up completed background processes
void cleanup_jobs()
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].job_id != -1)
        {
            int status;
            pid_t result = waitpid(jobs[i].pgid, &status, WNOHANG);
            
            if (result > 0)
            {
                // Process has terminated
                char *cmd_name = jobs[i].command;
                
                // Extract just the command name (first word)
                char temp_cmd[256];
                strncpy(temp_cmd, cmd_name, sizeof(temp_cmd) - 1);
                temp_cmd[sizeof(temp_cmd) - 1] = '\0';
                
                char *space = strchr(temp_cmd, ' ');
                if (space) {
                    *space = '\0';
                }
                
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                {
                    printf("%s with pid %d exited normally\n", temp_cmd, jobs[i].pgid);
                }
                else
                {
                    printf("%s with pid %d exited abnormally\n", temp_cmd, jobs[i].pgid);
                }
                
                // Remove the job
                remove_job(jobs[i].job_id);
            }
        }
    }
    
    // Also clean up old bg_pids array for backward compatibility
    for (int i = 0; i < job_count; i++)
    {
        if (bg_pids[i] != -1)
        {
            int status;
            pid_t result = waitpid(bg_pids[i], &status, WNOHANG);
            
            if (result > 0)
            {
                // Process has terminated
                char *cmd_name = bg_cmds[i];
                
                // Extract just the command name (first word)
                char *space = strchr(cmd_name, ' ');
                if (space)
                {
                    *space = '\0';  // Temporarily null-terminate
                    
                    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                    {
                        printf("%s with pid %d exited normally\n", cmd_name, bg_pids[i]);
                    }
                    else
                    {
                        printf("%s with pid %d exited abnormally\n", cmd_name, bg_pids[i]);
                    }
                    
                    *space = ' ';  // Restore the space
                }
                else
                {
                    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                    {
                        printf("%s with pid %d exited normally\n", cmd_name, bg_pids[i]);
                    }
                    else
                    {
                        printf("%s with pid %d exited abnormally\n", cmd_name, bg_pids[i]);
                    }
                }
                
                // Clean up the job entry
                free(bg_cmds[i]);
                bg_cmds[i] = NULL;
                bg_pids[i] = -1;
            }
        }
    }
}

// LLM GENERATED CODE ENDS