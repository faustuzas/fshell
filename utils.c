#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"

void print_greeting() {
    printf("Welcome to F-shell\n");
}

void print_goodbye() {
    printf("Have a nice day\n");
}

Status get_command(char *buff, size_t size) {
    printf("%s", PROMPT);
    if (fgets(buff, size, stdin) == NULL) {
        return NO_INPUT;
    }

    // check if command passed was not too long
    if (buff[strlen(buff) - 1] != '\n') {
        bool too_long = false;
        int ch;

        // clear stdout from left chars
        while (((ch = getchar()) != '\n') && (ch != EOF)) {
            too_long = true;
        }
        return too_long ? TOO_LONG : OK;
    }

    buff[strlen(buff) - 1] = '\0';
    return OK;
}

void print_error(Status status) {
    switch (status) {
        case NO_INPUT:
            fprintf(stderr, "\nError: No input provided\n");
            break;
        case TOO_LONG:
            fprintf(stderr, "\nError: Too long input\n");
            break;
        case FORKING_ERROR:
            fprintf(stderr, "\nError: Cannot fork process\n");
            break;
        case EXEC_ERROR:
            fprintf(stderr, "\nError: Cannot exec command\n");
            break;
        case WAIT_ERROR:
            fprintf(stderr, "\nError: Cannot wait for process\n");
            break;
        case TOO_MUCH_PROCESSES:
            fprintf(stderr, "\nError: Too much processes created\n");
            break;
        case SUSPENSION_ERROR:
            fprintf(stderr, "\nError: Cannot suspend process\n");
            break;
        case RESUMING_ERROR:
            fprintf(stderr, "\nError: Cannot resume process\n");
            break;
        case TERMINATION_ERROR:
            fprintf(stderr, "\nError: Cannot terminate process\n");
            break;
        default:
            break;
    }
}

void trim(char *str) {
    // empty string
    if (str == NULL || str[0] == '\0') {
        return;
    }

    size_t len = strlen(str);
    char *frontp = str;
    char *endp = str + len;

    // move front pointer to first non whitespace char
    while (isspace((unsigned char) *frontp)) {
        ++frontp;
    }

    if (endp != frontp) {
        // move end pointer to last non whitespace char
        while (isspace((unsigned char) *(--endp)) && endp != frontp) {}
    }

    // original string was only whitespace
    if (frontp != str && endp == frontp) {
        *str = '\0';
    } else if (str + len - 1 != endp) {
        // mark the new end of the string
        *(endp + 1) = '\0';
    }

    // shift the string so it starts at the original place
    char *new_frontp = str;
    if (frontp != new_frontp) {
        while (*frontp) {
            *new_frontp++ = *frontp++;
        }
        *new_frontp = '\0';
    }
}

char** parse_command(char *command) {
    size_t command_size = strlen(command);

    // find out how many separate commands there are
    int commands_count = 1;
    int index = 0;
    while (index < command_size) {
        if (command[index] == ' ') {
            commands_count++;
        }

        index++;
    }

    char** commands = malloc((commands_count + 1) * sizeof(char *));

    index = 0;
    int front_index = 0;
    int command_index = 0;
    while (index <= command_size) {
        if (index != command_size) {
            if (command[index] != ' ') {
                index++;
                continue;
            }
        }

        // allocate memory for string
        size_t size_of_command = index - front_index + 1;
        char *single_command = malloc(size_of_command * sizeof(char));

        memcpy(single_command, command + front_index, size_of_command);
        single_command[size_of_command - 1] = '\0';

        commands[command_index] = single_command;

        index++;
        front_index = index;
        command_index++;
    }

    commands[command_index] = NULL;

    return commands;
}

void free_commands(char** commands) {
    char* command;
    int index = 0;
    while ((command = commands[index++]) != NULL) {
        free((char*)command);
    }

    free(commands);
}

int find_null_index(char** commands) {
    int last_index = 0;
    while (commands[last_index] != NULL) {
        last_index += 1;
    }
    return last_index;
}

bool should_detach_process(char** commands) {
    int last_index = find_null_index(commands) - 1;

    // check if process should be detached
    if (last_index >= 1 && strncmp(commands[last_index], DETACH, strlen(DETACH)) == 0) {
        return true;
    }

    return false;
}

void clean_detachment_symbol(char** commands) {
    int detachment_symbol_index = find_null_index(commands) - 1;
    if (detachment_symbol_index <= 1) {
        return;
    }

    free(commands[detachment_symbol_index]);
    commands[detachment_symbol_index] = NULL;
}