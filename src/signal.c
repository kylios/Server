#include "mysignal.h"
#include "debug.h"
#include "logging.h"
#include "main.h"
#include "mysignal.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


/* Our signal handler functions */
static void sigchld_handler (int);
static void sigint_handler (int);

/* Declared in main.c and indicates whether the main loop should 
 * continue to run or not.  Set to FALSE to gracefully exit the 
 * application. */
extern int run;

int
init_signal_handler (int sig)
{
    /* Here we map signals we want to catch to respective functions that can
     * handle them */
    signal (SIGCHLD, sigchld_handler);
    signal (SIGINT, sigint_handler);

    return 0;
};


/* Called when a child process exits.  We need to remove the child from our
 * index and invoke any listeners that listen on this event.  To do this, we
 * will queue up an event to tell the master server to do this so that we can
 * return from the signal handler ASAP */
static void 
sigchld_handler (int sig)
{
    ASSERT (sig == SIGCHLD);

    /* Get the pid and status of the child that exited */
    int status;
    pid_t child = wait (&status);

    /* Now check the status code and act accordingly */

    printf ("Child exited: %d \n", child);
};

static void
sigint_handler (int sig)
{
    server_log ("SIGINT received, exiting... \n");

    /* Exit gracefully... */
    run = false;
};


