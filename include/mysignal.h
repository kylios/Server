#ifndef SIGNAL_H
#define SIGNAL_H

#include <signal.h>

/* Initializes our signal handler function by setting up the necessary data
 * structures and mappings for handling signals */
int init_signal_handler ();

/* Handles signals sent to the server process */
void signal_handler (int sig);

#endif //SIGNAL_H

