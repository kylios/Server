#ifndef CHILD_H
#define CHILD_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <semaphore.h>
#include <stdio.h>

/**
 * This structure defines a child record in the server's data index.  A child
 * has several pieces of data associated with it: 
 * - socket file descriptor for communitation
 * - ip address
 * - port
 * - protocol (ipv4, ipv6)
 * */
struct server_child
{
    pid_t pid;          // child's pid
    int ourid;          // ID assigned by the server
    int clientfd;       // the child's connection with the client
    int parentread;     // parent uses this to read
    int parentwrite;    // parent uses this to write
    sem_t* logsem;
    sem_t* errsem;
    FILE* logfile;
    FILE* errfile;
};

char** build_child_argv (char* exe, char* scriptname, int newfd, int childread, int childwrite);

#endif // CHILD_H
