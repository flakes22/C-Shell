// LLM GENERATED CODE BEGINS
#include "../include/header.h"

int add_job(pid_t pid, char *command, int status) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].job_id == -1) {
            jobs[i].job_id = i + 1;
            jobs[i].pgid = pid;
            jobs[i].status = status;
            strncpy(jobs[i].command, command, sizeof(jobs[i].command) - 1);
            jobs[i].command[sizeof(jobs[i].command) - 1] = '\0';
            if (i >= job_count_global) {
                job_count_global = i + 1;
            }
            return jobs[i].job_id;
        }
    }
    return -1; // No space for new job
}

void remove_job(int job_id) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].job_id == job_id) {
            jobs[i].job_id = -1;
            jobs[i].pgid = -1;
            jobs[i].status = -1;
            memset(jobs[i].command, 0, sizeof(jobs[i].command));
            break;
        }
    }
}

Job* find_job_by_pid(pid_t pid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].job_id != -1 && jobs[i].pgid == pid) {
            return &jobs[i];
        }
    }
    return NULL;
}

Job* find_job_by_id(int job_id) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].job_id == job_id) {
            return &jobs[i];
        }
    }
    return NULL;
}

Job* get_most_recent_job() {
    int highest_id = -1;
    Job* most_recent = NULL;
    
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].job_id != -1 && jobs[i].job_id > highest_id) {
            highest_id = jobs[i].job_id;
            most_recent = &jobs[i];
        }
    }
    return most_recent;
}

int cmp_jobs(const void *a, const void *b)
{
    int idx_a = *(const int*)a;
    int idx_b = *(const int*)b;
    
    // Compare command names lexicographically
    if (bg_cmds[idx_a] && bg_cmds[idx_b])
    {
        return strcmp(bg_cmds[idx_a], bg_cmds[idx_b]);
    }
    
    // Handle NULL cases
    if (!bg_cmds[idx_a] && !bg_cmds[idx_b]) return 0;
    if (!bg_cmds[idx_a]) return 1;
    if (!bg_cmds[idx_b]) return -1;
    
    return 0;
}
// LLM GENERATED CODE BEGINS