#ifndef PROCESSES_H
#define PROCESSES_H

#include <sys/types.h>
#include "utils.h"

Status add_process(pid_t pid, char* command);
Status remove_process(pid_t pid);
Status resume_process(pid_t pid);
Status suspend_process(pid_t pid);

pid_t get_last_suspended_process();
bool is_process_suspended();

void clean_processes();
void print_processes();

#endif