#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "processes.h"

#define p_status int
#define RUNNING 0
#define SUSPENDED 1

#define MAX_PROCESSES 10

typedef struct process {
    pid_t pid;
    p_status status;
    char* name;
} Process;

Process* processes[MAX_PROCESSES] = {NULL };
pid_t last_suspended_process = -1;

pid_t get_last_suspended_process() {
    return last_suspended_process;
}

int next_index() {
    for (int i = 0; i < MAX_PROCESSES; ++i) {
        if (processes[i] == NULL) {
            return i;
        }
    }

    return -1;
}

char* named_status(p_status status) {
    switch (status) {
        case SUSPENDED:
            return "SUSPENDED";
        case RUNNING:
            return "RUNNING";
        default:
            return "UNRECOGNIZED";
    }
}

bool is_process_suspended(pid_t pid) {
    for (int i = 0; i < MAX_PROCESSES; ++i) {
        Process* p = processes[i];
        if (p != NULL && p->pid == pid && p->status == SUSPENDED) {
            return true;
        }
    }

    return false;
}

void clean_process(Process* p) {
    if (p != NULL) {
        free(p->name);
        free(p);
    }
}

void clean_processes() {
    for (int i = 0; i < MAX_PROCESSES; ++i) {
        Process* p = processes[i];
        if (p == NULL) {
            continue;
        }

        clean_process(p);
    }
}

Status add_process(pid_t pid, char* command) {
    int index = next_index();
    if (index == -1) {
        return TOO_MUCH_PROCESSES;
    }

    Process* p = malloc(sizeof(processes));
    p->pid = pid;
    p->status = RUNNING;
    p->name = malloc(sizeof(char) * strlen(command));
    strcpy(p->name, command);

    processes[index] = p;

    return OK;
}

Status suspend_process(pid_t pid) {
    if (kill(pid, SIGTSTP) == 0) {
        for (int i = 0; i < MAX_PROCESSES; ++i) {
            Process* p = processes[i];
            if (p != NULL && p->pid == pid) {
                p->status = SUSPENDED;
                break;
            }
        }

        last_suspended_process = pid;

        return OK;
    }

    return SUSPENSION_ERROR;
}

Status resume_process(pid_t pid) {
    if (kill(pid, SIGCONT) == 0) {
        for (int i = 0; i < MAX_PROCESSES; ++i) {
            Process* p = processes[i];
            if (p != NULL && p->pid == pid) {
                p->status = RUNNING;
                break;
            }
        }

        return OK;
    }

    return RESUMING_ERROR;
}

void remove_process(pid_t pid) {
    if (last_suspended_process == pid) {
        last_suspended_process = -1;
    }

    for (int i = 0; i < MAX_PROCESSES; ++i) {
        Process* p = processes[i];
        if (p != NULL && p->pid == pid) {
            clean_process(p);
            processes[i] = NULL;
        }
    }
}

Status kill_process(pid_t pid) {
    if (last_suspended_process == pid) {
        last_suspended_process = -1;
    }

    if (kill(pid, SIGINT) == 0) {
        remove_process(pid);
        return OK;
    }

    return TERMINATION_ERROR;
}

void print_processes() {
    printf("%6s | %10s | %15s\n", "PID", "STATUS", "NAME");
    for (int i = 0; i < MAX_PROCESSES; ++i) {
        Process* p = processes[i];
        if (p == NULL ) {
            continue;
        }

        printf("%6d | %10s | %15s\n", p->pid, named_status(p->status), p->name);
    }
}


