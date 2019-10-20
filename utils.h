#ifndef UTILS_H
#define UTILS_H

#define bool int
#define true 1
#define false 0

#define Status int
#define OK 0
#define NO_INPUT 1
#define TOO_LONG 2
#define FORKING_ERROR 3
#define EXEC_ERROR 4
#define WAIT_ERROR 5

#define PROMPT ">"
#define DETACH "&"

#define for_ever while(1)

#define CMD_EXIT "exit"
#define CMD_CHDIR "cd"

#define CHILD_PROCESS 0

Status get_command(char *buff, size_t size);
void print_error(Status status);
void print_greeting();
void exit_shell();
void trim(char *str);
char** parse_command(char *command);
void free_commands(char** commands);
void print_goodbye();
bool should_detach_process(char** commands);
void clean_detachment_symbol(char** commands);

#endif