#ifndef UTILS_H
#define UTILS_H

#include <string.h>

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
#define TOO_MUCH_PROCESSES 6
#define SUSPENSION_ERROR 7
#define RESUMING_ERROR 8
#define TERMINATION_ERROR 9

#define PROMPT ">"
#define DETACH "&"

#define for_ever while(1)

#define CMD_EXIT "exit"
#define CMD_CHDIR "cd"
#define CMD_FG "fg"
#define CMD_BG "bg"
#define CMD_JOBS "jobs"

#define CHILD_PROCESS 0

#define streq(X, Y) strncmp((X), (Y), strlen((Y))) == 0

Status get_command(char *buff, size_t size);
void print_error(Status status);
void print_greeting();
void trim(char *str);
char** parse_command(char *command);
void free_commands(char** commands);
void print_goodbye();
bool should_detach_process(char** commands);
void clean_detachment_symbol(char** commands);
int find_null_index(char** commands);

#endif