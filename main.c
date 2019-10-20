#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<signal.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "utils.h"

#define COMMAND_BUFF_SIZE 256
#define MAX_SUSPENDED_PROCESSES 10

typedef struct process {
    pid_t pid;
    int pipe_fds[2];
    p_status status;
} process;

pid_t process_in_fg = -1;
pid_t last_suspended_process = -1;

process* suspended_processes[MAX_SUSPENDED_PROCESSES] = { NULL };

int next_index_for_suspended() {
    for (int i = 0; i < MAX_SUSPENDED_PROCESSES; ++i) {
        if (suspended_processes[i] == NULL) {
            return i;
        }
    }

    return -1;
}

bool is_process_suspended(pid_t pid) {
    for (int i = 0; i < MAX_SUSPENDED_PROCESSES; ++i) {
        Process* p = suspended_processes[i];
        if (p != NULL && p->pid == pid) {
            return true;
        }
    }

    return false;
}

void remove_suspended_process(pid_t pid) {
    for (int i = 0; i < MAX_SUSPENDED_PROCESSES; ++i) {
        Process* p = suspended_processes[i];
        if (p != NULL && p->pid == pid) {
            free(p);
            suspended_processes[i] = NULL;
        }
    }
}

void free_suspended_processes() {
    for (int i = 0; i < MAX_SUSPENDED_PROCESSES; ++i) {
        free(suspended_processes[i]);
    }
}

void signal_handler(int sig) {
    switch(sig) {
      case SIGINT:
            if (process_in_fg > 0) {
                kill(process_in_fg, SIGINT);
            }
            break;
        case SIGTSTP:
            if (process_in_fg > 0) {
                int index = next_index_for_suspended();
                if (index == -1) {
                    printf("Only %d processed can be suspended\n", MAX_SUSPENDED_PROCESSES);
                    break;
                }

                Process* p = malloc(sizeof(Process));
                p->pid = process_in_fg;
                suspended_processes[index] = p;

                last_suspended_process = process_in_fg;

                kill(process_in_fg, SIGTSTP);
            }
            break;
    }
}

void init_handler(){
    struct sigaction sa;

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if(sigaction(SIGINT, &sa, NULL) == -1)
        printf("Couldn't catch SIGINT - Interrupt Signal\n");
    if(sigaction(SIGTSTP, &sa, NULL) == -1)
        printf("Couldn't catch SIGTSTP - Suspension Signal\n");
}

void wait_process(pid_t pid) {
    // WUNTRACED used to stop waiting when suspended
    int status;
    waitpid(pid, &status, WUNTRACED);

    if (WIFEXITED(status)) {
        printf("Exited with %d\n", WEXITSTATUS(status));
    } else if(WIFSIGNALED(status)){
        printf("Process received SIGNAL %d\n", WTERMSIG(status));
    }
}

int main() {
    init_handler();

    print_greeting();

    Status status;
    char command_buff[COMMAND_BUFF_SIZE];
    for_ever
    {
        process_in_fg = -1;

        status = get_command(command_buff, COMMAND_BUFF_SIZE);
        if (status != OK) {
            print_error(status);
        }

        trim(command_buff);
        if (strlen(command_buff) < 1) {
            continue;
        }

        char **parsed_commands = parse_command(command_buff);
        if (strncmp(parsed_commands[0], CMD_EXIT, strlen(CMD_EXIT)) == 0) {
            free_commands(parsed_commands);
            print_goodbye();
            return 0;
        }

        if (strncmp(parsed_commands[0], CMD_CHDIR, strlen(CMD_CHDIR)) == 0) {
            chdir(parsed_commands[1]);
            free_commands(parsed_commands);
            continue;
        }

        if (streq(parsed_commands[0], CMD_FG)) {
            int args_count = find_null_index(parsed_commands);
            if (args_count != 1 && args_count != 2) {
                free_commands(parsed_commands);
                printf("Usage of %s: %s [pid]\n", CMD_FG, CMD_FG);
                continue;
            }

            pid_t pid;
            if (args_count == 1) {
                pid = last_suspended_process;
            }

            if (args_count == 2) {
                pid = atoi(parsed_commands[1]);
            }

            process_in_fg = pid;
            if (is_process_suspended(pid)) {
                kill(pid, SIGCONT);
                remove_suspended_process(pid);
            }

            wait_process(pid);
            continue;
        }

        if (streq(parsed_commands[0], CMD_FG)) {
            if (find_null_index(parsed_commands) != 2) {
                free_commands(parsed_commands);
                printf("Usage of %s: %s <pid>\n", CMD_FG, CMD_FG);
            }
            pid_t pid = atoi(parsed_commands[1]);
            process_in_fg = pid;

            if (is_process_suspended(pid)) {
                kill(pid, SIGCONT);
            }

            wait_process(pid);
            continue;
        }

        bool should_detach = should_detach_process(parsed_commands);
        if (should_detach) {
            clean_detachment_symbol(parsed_commands);
        }

        pid_t pid = fork();
        if (pid == -1) {
            print_error(FORKING_ERROR);
        } else if (pid == CHILD_PROCESS) {
            setpgid(0, 0);

            // if process detached, silence him
            if (should_detach) {
                // int fd = open("/dev/null", O_WRONLY);
                // dup2(fd, 1);    /* make stdout a copy of fd (> /dev/null) */
                // dup2(fd, 2);    /* ...and same with stderr */
                // close(fd);      /* close fd */
            }

            execvp(parsed_commands[0], (char *const *)parsed_commands);
            print_error(EXEC_ERROR);
            exit(1);
        } else if(pid > 0) {
            process_in_fg = pid;
            setpgid(pid, pid);

            if (should_detach) {
                printf("Detached process id: %d\n", pid);
            } else {
                wait_process(pid);
            }
        }
    }
}