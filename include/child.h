#ifndef CHILD_H
#define CHILD_H

#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>

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
    int clientfd;       // the child's connection with the client
    int parentread;     // parent uses this to read
    int parentwrite;    // parent uses this to write
};

char** build_child_argv ();

#endif // CHILD_H
