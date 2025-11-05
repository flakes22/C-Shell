#include "../include/header.h"

// Define the global variables
pid_t fg_pgid = 0;
char *tokens[256];
int tok_count = 0;
int pos = 0;
int check_pos = 0;
pid_t bg_pids[101];
char *bg_cmds[101];
int job_count = 0;
char command_log[MAX_LOG_SIZE][256];
int log_count = 0;
int executing_from_log = 0;
char fg_cmd[256];
Job jobs[MAX_JOBS];
int job_count_global = 0;
pid_t current_fg_pid = 0;  // Current foreground process PID

// Directory variables
char cwd[1024];
char home_dir[1024];
char pwd[1024];

// Initialize arrays to prevent garbage values
void init_globals() {
    for (int i = 0; i < 101; i++) {
        bg_pids[i] = -1;
        bg_cmds[i] = NULL;
    }
    for (int i = 0; i < 256; i++) {
        tokens[i] = NULL;
    }
    for (int i = 0; i < MAX_JOBS; i++) {
        jobs[i].job_id = -1;
        jobs[i].pgid = -1;
        jobs[i].status = -1;
        memset(jobs[i].command, 0, sizeof(jobs[i].command));
    }
}