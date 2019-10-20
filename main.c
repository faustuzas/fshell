#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "utils.h"

#define COMMAND_BUFF_SIZE 256

int main()
{
    print_greeting();

    Status status;
    char command_buff[COMMAND_BUFF_SIZE];
    for_ever
    {
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

        bool should_detach = should_detach_process(parsed_commands);
        if (should_detach) {
            clean_detachment_symbol(parsed_commands);
        }

        pid_t pid = fork();
        if (pid == -1) {
            print_error(FORKING_ERROR);
        } else if (pid == CHILD_PROCESS) {
            if (execvp(parsed_commands[0], (char *const *)parsed_commands) == -1) {
                print_error(EXEC_ERROR);
                exit(1);
            }
        } else {
            free_commands(parsed_commands);
            if (waitpid(pid, &status, 0) == -1) {
                print_error(WAIT_ERROR);
            }
        }
    }
}