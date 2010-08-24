#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

#define _XOPEN_SOURCE 600

int init_logging (char* logfile_path, char* errfile_path);

void set_log_child (pid_t pid);

void server_log (const char* format, ...);
void server_err (const char* format, ...);
void print_err (int);

void end_logging ();

#endif //LOGGING_H

