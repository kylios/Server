#ifndef SERVER_H
#define SERVER_H

#include <signal.h>

/**
 * Bind to local port PORT for listening.  Returns -1 if failure. 
 * \param port The local port on which to listen
 * \param ipver The ip version to use
 * \param *sockfd pointer to the variable in which the socket descriptor will be stored
 * \return -1 on failure, 0 on success
 * */
int bind_to_localhost (const char* port, int ipver, int* sockfd);

/**
 * Initializes the signal handlers.  Since we may encounter several different
 * signals during the lifetime of the server, it is important we be able to exit
 * cleanly and gracefully. 
 * \param *sa pointer to a struct sigaction
 * \return -1 on failure, 0 on success
 * */
int init_signal_handlers (struct sigaction* sa);



#endif // SERVER_H

