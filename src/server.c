/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

//#include "server.h"


/* Ideally, this information will be read from a configuration file.  For now
 * though, we'll just leave them as definitions at the top here. */
#define PORT        "3490"  // the port users will be connecting to
#define BACKLOG     10	    // how many pending connections queue will hold
#define IPVER       4       // The ip version to use (4 or 6)

void sigchld_handler(int s)
{
	while (waitpid (-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr (struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char** argv)
{
    /* The connection on which we will listen */
	int sockfd;
    /* This is used for new connections */
    int new_fd;
    /* Connector's addres sinformation */
	socklen_t sin_size;

    /* Signal handlers */
	struct sigaction sa;

    /* The ip address in a string */
	char s[INET6_ADDRSTRLEN];

    /* Try to set up our signal handlers */
    if (-1 == init_signal_handlers (&sa))
    {
        fprintf (stderr, "Error initializing signal handler \n");
        return 1;
    }

    /* Bind and get our socket descriptor */
    if (bind_to_localhost (PORT, 4, &sockfd) == -1)
    {
        fprintf (stderr, "Error binding to localhost \n");
        return 1;
    }

    if (listen (sockfd, BACKLOG) == -1) 
    {
	    fprintf (stderr, "Error listening on socket \n");
		return 1;
	}

	printf("server: waiting for connections...\n");

    /* This is the main loop where we will be accepting new connections.  Each
     * time a connection is accepted, a new process is spawned to handle that
     * connection.  This way we can ensure maximum responsiveness for each
     * client. */
	while(1) 
    {  
		sin_size = sizeof their_addr;
        /* Accept the connection.  Will block until a request actually comes in.
         * */
		new_fd = accept (sockfd, (struct sockaddr*) &their_addr, &sin_size);
		if (new_fd == -1) 
        {
			perror ("accept");
			continue;
		}

		inet_ntop (their_addr.ss_family,
			get_in_addr ((struct sockaddr*) &their_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);

        /* Fork a child process to handle this new connection. */
        pid_t pid = fork ();
		if (!pid) 
        {
            /* This is the child process */
			close (sockfd); // child doesn't need the listener
			if (send (new_fd, "Hello, world!", 13, 0) == -1)
				perror ("send");
			close (new_fd);
			exit (0);
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}

int
bind_to_localhost (const char* port, int ipver, int* sockfd)
{
	struct addrinfo hints, *servinfo, *p;
    
    /* Set up the addrinfo struct to
     * -Use ipv4 or ipv6, whichever was specified
     * -Use a stream socket
     * -Use passive mode 
     * */
    memset (&hints, 0, sizeof hints);
	hints.ai_family = (ipver == 4 ? AF_INET : AF_INET6);
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; 

	int rv;
	if ((rv = getaddrinfo (NULL, port, &hints, &servinfo)) != 0) {
		fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (rv));
		return -1;
	}

    /* Find an address to bind to.  Since we just need to bind to ourselves,
     * this should not fail. 
     * */
    int yes = 1;
    int fd;
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((fd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof (int)) == -1) {
			perror("setsockopt");
			return -1;
		}

		if (bind (fd, p->ai_addr, p->ai_addrlen) == -1) {
			close(fd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return -1;
	}

	freeaddrinfo(servinfo); // all done with this structure

    /* This is the socket we'll return to the caller */
    *sockfd = fd;

    return 0;
}

int
init_signal_handlers (struct sigaction* sa)
{
	sa->sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset (&sa->sa_mask);
	sa->sa_flags = SA_RESTART;
	if (sigaction (SIGCHLD, sa, NULL) == -1) 
    {
		return -1;
	}

    return 0;
}
