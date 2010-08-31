#ifndef CHILD_H
#define CHILD_H

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <semaphore.h>
#include <stdio.h>

#include "bst.h"

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

    pthread_t thread;  // This child's communication thread
    pthread_mutex_t init_lock;// Lock that signals the thread is ready to start
};

/**
 * Defines a data structure based on a BST for maintaining an index of all the
 * child processes that are running.  We must be able to query the index via the
 * child's proc_id and get a struct describing the child, namely struct
 * SERVER_CHILD.  This structure will need to support insertion, deletion, and
 * find. */
struct child_index
{
    struct bst_tree tree;    // TODO use avl or more efficient structure instead
};

void init_child_index ();
int child_compare (const void* child1, const void* child2, const void* AUX);
void child_dump (const void* child);
void add_child (struct server_child* child);
struct server_child* remove_child (int our_id);
struct server_child* get_child (int proc_id);
void dump_child_index ();


/**
 * The function executed in the child's communications thread.  This thread's
 * sole purpose is to handle communication with a single child process.  This is
 * handled in its own thread to ensure responsiveness with the server.
 * */
void* child_comm_thread (void* child);



/**
 * Builds the argv array for a new child process.  All required fields must be
 * passed in here as parameters, and this function will allocate space for an
 * array and fill it accordingly. */
char** build_child_argv (const char* exe, char* scriptname, int newfd, 
        int childread, int childwrite);

;
#endif // CHILD_H
