#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<signal.h>
#include <sys/wait.h>

#include "utils.h"

#define COMMAND_BUFF_SIZE 256

pid_t process_in_fg = 0;
void signal_handler(int sig) {
    printf("FG Process id: %d\n", process_in_fg);
    switch(sig) {
      case SIGINT:
            printf("SIGINT received \n");
            if (process_in_fg > 0) {
                kill(process_in_fg, SIGINT);
            }
            break;
        case SIGTSTP:
            printf("SIGTSTP received \n");
            if (process_in_fg > 0) {
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

void handler(int sig) {
    printf("Received, yeaaah\n");
}

int main() {

    pid_t pid = fork();
    if (pid == 0) {
        signal(SIGINT, handler);
        char* cmds[] = { "./work.sh", NULL };
        if (execvp(cmds[0], cmds) == -1) {
            print_error(EXEC_ERROR);
            exit(1);
        }
    } else {
        sleep(5);
        kill(pid, SIGINT);
    }

    return 0;



    init_handler();

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
        } else if(pid > 0){ //Parent
            process_in_fg = pid;

            setpgid(pid, pid);

            // Waits if background flag not activated.
            if(!should_detach){
                // WUNTRACED used to stop waiting when suspended
                waitpid(pid, &status, WUNTRACED);

                if (WIFEXITED(status)) {
                    printf("Exited with %d\n", WEXITSTATUS(status));
                } else if(WIFSIGNALED(status)){
                    printf("Process received SIGNAL %d\n", WTERMSIG(status));
                }
            }
        }
    }
}