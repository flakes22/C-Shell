#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <regex.h>

// ---- Global defines ----
#define MAX_LOG_SIZE 15
#define MAX_JOBS 100

// Job management structure
typedef struct {
    int job_id;
    pid_t pgid;
    char command[256];
    int status; // 0=running, 1=stopped
} Job;

// ---- Global variables (extern only, defined in a .c file) ----
extern pid_t fg_pgid;   
extern char *tokens[256];
extern int tok_count;
extern int pos;
extern int check_pos;
extern pid_t bg_pids[101];
extern char *bg_cmds[101];
extern int job_count;

// Log-related global variables
extern char command_log[MAX_LOG_SIZE][256];
extern int log_count;
extern int executing_from_log;
extern char fg_cmd[256];

// Directory variables
extern char cwd[1024];
extern char home_dir[1024];
extern char pwd[1024];

extern Job jobs[MAX_JOBS];
extern int job_count_global;
extern pid_t current_fg_pid;  // Current foreground process PID

// ---- Function prototypes ----
int tokenizer(char *input);
int valid_name(char *input);
void init_globals();

// Command handlers
void handle_hop(char *input, char *cwd, char *home_dir, char *pwd);
void handle_reveal(char *input, char *home_dir, char *pwd);
void handle_log(char *input, char *home_dir, char *pwd);
void handle_activities();
void handle_ping(char **tokens);
void handle_fg(char *input);
void handle_bg(char *input);

// Command execution
void execute_sequential(char *input);
void execute_command(char *input, int background);

// Job management
void cleanup_jobs();
int cmp_jobs(const void *a, const void *b);
int add_job(pid_t pid, char *command, int status);
void remove_job(int job_id);
Job* find_job_by_pid(pid_t pid);
Job* find_job_by_id(int job_id);
Job* get_most_recent_job();

// Signal handling
void setup_signal_handlers();
void sigint_handler(int sig);
void sigtstp_handler(int sig);

// Pipe syntax validation
int has_invalid_pipe_syntax(char *input);

// Log management functions
void add_to_log(char *command);
void save_log_to_file(char *home_dir);
void load_log_from_file(char *home_dir);

#endif