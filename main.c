#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "utils.h"
#include "processes.h"

#define COMMAND_BUFF_SIZE 256

pid_t process_in_fg = -1;

void signal_handler(int sig) {
    switch (sig) {
        case SIGINT:
            if (process_in_fg > 0) {
                check(kill_process(process_in_fg));
            }
            break;
        case SIGTSTP:
            if (process_in_fg > 0) {
                check(suspend_process(process_in_fg));
                break;
            }
    }
}

void init_handler() {
    struct sigaction sa;

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);
}

void wait_process(pid_t pid) {
    int status;
    waitpid(pid, &status, WUNTRACED);
    if (WIFEXITED(status)) {
        remove_process(pid);
    } else if(WIFSIGNALED(status)){
        printf("Process received SIGNAL %d\n", WTERMSIG(status));
    }
}

int main() {
    init_handler();

    print_greeting();

    Status status;
    char command_buff[COMMAND_BUFF_SIZE];
    for_ever {
        status = get_command(command_buff, COMMAND_BUFF_SIZE);
        if (status != OK) {
            print_error(status);
        }

        trim(command_buff);
        if (strlen(command_buff) < 1) {
            continue;
        }

        char **parsed_commands = parse_command(command_buff);
        if (streq(parsed_commands[0], CMD_EXIT)) {
            free_commands(parsed_commands);
            clean_processes();
            print_goodbye();
            return 0;
        }

        if (streq(parsed_commands[0], CMD_CHDIR)) {
            int args_count = find_null_index(parsed_commands);
            if (args_count == 2) {
                chdir(parsed_commands[1]);
            } else {
                printf("Usage of %s: %s dir\n", CMD_CHDIR, CMD_CHDIR);
            }

            free_commands(parsed_commands);
            continue;
        }

        if (streq(parsed_commands[0], CMD_FG) || streq(parsed_commands[0], CMD_BG)) {
            char* cmd = parsed_commands[0];
            int args_count = find_null_index(parsed_commands);

            pid_t pid;
            if (args_count == 1) {
                pid = get_last_suspended_process();
            } else if (args_count == 2) {
                pid = strtol(parsed_commands[1], NULL, 0);
            } else {
                free_commands(parsed_commands);
                printf("Usage of %s: %s [pid]\n", cmd, cmd);
                continue;
            }

            if (pid == -1) {
                free_commands(parsed_commands);
                continue;
            }

            printf("PID: %d\n", pid);
            if (is_process_suspended(pid)) {
                check(resume_process(pid));
            }

            if (streq(cmd, CMD_FG)) {
                process_in_fg = pid;
                wait_process(pid);
            } else {
                process_in_fg = -1;
            }

            free_commands(parsed_commands);
            continue;
        }

        if (streq(parsed_commands[0], CMD_JOBS)) {
            print_processes();

            free_commands(parsed_commands);
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
            execvp(parsed_commands[0], (char* const *)parsed_commands);
            print_error(EXEC_ERROR);
            exit(1);
        } else if(pid > 0) {
            setpgid(pid, pid);
            process_in_fg = pid;
            check(add_process(pid, command_buff));
            if (should_detach) {
                process_in_fg = -1;
                printf("Detached process id: %d\n", pid);
            } else {
                wait_process(pid);
            }
        }
    }
}