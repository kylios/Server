
// Standard types
#include <sys/types.h>
#include <sys/stat.h>
// Sockets library
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
// Standard errors
#include <errno.h>
// For system calls
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include "main.h"
#include "defaults.h"
#include "mysignal.h"
#include "debug.h"
#include "logging.h"
#include "type.h"
#include "child.h"

/* Parse the configuration file and set the options as our global program
 * options, overwriting any default options */
static void parse_config_file ();

/* Parse the command line options and set them as our global program options,
 * overwriting any default or config file options */
static void parse_command_line (int argc, char** argv);

/* Reads our environment variables and sets certain options accordingly */
static void read_env_variables (char** envp);

/* Prepares the application to become a daemon.  This is fairly simple, but
 * there are several steps involved:
 * -Fork off the parent process
 * -Change file mode mask (umask)
 * -Create a unique session ID (SID)
 * -Change the current working directory to a safe place
 * -Close standard file descriptors
 * */
static void daemonize_prep ();

/* Initialize the default program options */
static void init_defaults ();

/* Bind to localhost so that we can start listening for client connections */
static int bind_to_localhost (const char* port, int ipver, int* sockfd);

/* Exits the program gracefully.  Should only be called by a child process */
static void exit_child (int status);

/* Global program options.  These get read both from the command line and from a
 * configuration file */
static struct options global_options;

/* An array of interpreter pathnames that we pass to execve when a child
 * process is created */
static char** interpreters;

/* Socket descriptors for the connection on which we listen and on which new
 * client connections are made. */
static int sockfd, newfd;

/* The next process ID to assign */
static int procid = 1;

/* Set to false to quit */
bool run = true;

int
main (int argc, char** argv, char** envp)
{
    printf ("server by Kyle Racette \n");

    init_defaults ();

    /* Read the environment variable and set certain options accordingly*/
    read_env_variables (envp);

    /* Read configuration file */
    parse_config_file ();

    /* Read command line */
    parse_command_line (argc, argv);

    /* Parse code (python, perl, or custom) */

    /* Prepare to be a daemon */
    //daemonize_prep (); 

    /* Open our log files */
    if (-1 == init_logging (global_options.logfile_path, global_options.errfile_path))
    {
        perror ("Failed to initialize our logs");
        exit_program (EXIT_FAILURE);
    };
    server_log ("Logs initialized...");

    /* Initialize our system environment */

    /* Initialize our signal handlers */
    init_signal_handler ();

    /* Set up the network to listen for clients */
    struct sockaddr_storage client_addr;    // Client's address info
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];           // ip address as a string
	struct sockaddr_storage their_addr; 

    if (-1 == bind_to_localhost (global_options.port, global_options.ipver, 
                &sockfd))
    {
        server_err ("Error binding to localhost");
        print_err (errno);
        exit_program (EXIT_FAILURE);
    }

    if (-1 == listen (sockfd, global_options.backlog))
    {
        server_err ("Error listening on socket");
        print_err (errno);
        exit_program (EXIT_FAILURE);
    }

    server_log ("waiting for client connections...");

    /* Loop to start listening for client connections */
    while (run)   
    {
        sin_size = sizeof their_addr;

        /* Block until a connection is received */
        newfd = accept (sockfd, (struct sockaddr*) &their_addr, &sin_size);
        if (-1 == newfd)
        {
            server_err ("Failed to accept connection attempt");
            print_err (errno);    
            continue;
        }

        // TODO: probably will need to get the client's connection information..
        // like IP address and all...

        server_log ("got connection");

        /* 
         * parent reads from readpipe[0]
         * child writes to readpipe[1]
         * child reads from writepipe[0]
         * */
        int writepipe[2] = {-1, -1};
        int readpipe[2] = {-1, -1};

        if (pipe (readpipe) < 0 || pipe (writepipe) < 0)
        {
            server_err ("Could not create pipes for child. Terminating");
            run = false;
        }

        int childread = writepipe[0];
        int childwrite = readpipe[1];
        int parentread = readpipe[0];
        int parentwrite = writepipe[1];

        struct server_child* new_child = 
               (struct server_child*) malloc (sizeof (struct server_child));

        if (new_child == NULL)
        {
            server_err ("Could not allocate space for a new child record");
            
            close (childread);
            close (childwrite);
            close (parentread);
            close (parentwrite);

            /* Just continue... maybe more memory will free up, but this 
             * should not be a show-stopper. */
            continue;
        }


        /* Keep a record of this child here */
        new_child->ourid = procid++;
        new_child->clientfd = newfd;
        new_child->parentread = parentread;
        new_child->parentwrite = parentwrite;

        /* Fork a child for this connection */
        pid_t pid = fork ();
        if (pid < 0)
        {
            free (new_child);
            server_err ("Could not fork a child! Terminating server...");
            run = false;
        }
        else if (pid == 0)
        {
            free (new_child);
            /* Child */

            /* Don't need this since it was used for listening for new
             * connections */
            close (sockfd);

            /* Close the parent's pipes since we won't need them here */
            close (parentwrite);
            close (parentread);

            /* Set logging to write this child's pid in front of all
             * messages */
            set_log_child (getpid ());

            /* Grab the executable name */
            const char* exe = interpreters[global_options.interpreter];

            /* Build the argument list.  Here we will need to pass the
             * values of all our file descriptors so they are accessible
             * from the child script */
            char** _argv = build_child_argv (exe, global_options.script_path, 
                    newfd, childread, childwrite);

            /* The environment array.  This is simply the same that was
             * passed to our main function. */
            char** _envp = envp;

            server_log ("Dumping our child's variables...");
            server_log ("Executable: %s", exe);
            server_log ("newfd: %d", newfd);
            server_log ("childread: %d", childread);
            server_log ("childwrite: %d", childwrite);

            /* Close all our logging resources */
            end_logging ();

            /* Initialize the child and get it started doing it's thing.
             * If that routine returns, then kill the process because
             * it means there was an error. */
            if (-1 == execve (exe, _argv, _envp))
            {
                /* Start logging again so we can report our status */
                init_logging (global_options.logfile_path,
                        global_options.errfile_path);
                server_err ("Failed to call exec:");
                print_err (errno);

                /* Kill the child process */
                exit_child (EXIT_FAILURE);
            }
            NOT_REACHED
        }
        else
        {
            /* Parent */

            /* We don't need this resource anymore */
            close (newfd);

            /* Close the child's pipes */
            close (childread);
            close (childwrite);

            new_child->pid = pid;
            
            /* Do something with this record */
            // TODO REMOVE !!!
            free (new_child); 
        }
    }
    
    


    exit_program (EXIT_SUCCESS);
    printf ("END \n");
}

static void 
parse_config_file ()
{
    FILE* cfd = fopen (global_options.config_path, "r");
    if (cfd == NULL)
    {
        server_err ("could not open configuration file at `%s'", 
                global_options.config_path);
        exit (EXIT_FAILURE);
    }


};

static void 
parse_command_line (int argc, char** argv)
{

};

static void
daemonize_prep ()
{
    pid_t pid, sid;

    /* First, fork the process and then kill the parent */
    pid = fork ();
    if (pid < 0)
    {
        /* Error */
        exit (EXIT_FAILURE);
    }
    if (pid > 0)
    {
        /* Parent */
        exit (EXIT_SUCCESS);
    }
    ASSERT (pid == 0);

    /* Change the file umask */
//    umask (atoi (global_options.umask));
    
    /* Create a new session id for the process so this process does not become
     * an orphan */
    sid = setsid ();
    if (sid < 0)
    {
        exit (EXIT_FAILURE); 
    }

    /* Change the CWD to / */
    if ((chdir ("/")) < 0)
    {
        exit (EXIT_FAILURE);
    }

    /* Close all open file descriptors, including stdin, stdout, and stderr (0,
     * 1, and 2).  This is a security measure; if the parent process had open
     * files previously, they would have been inherited by this daemon process.
     * We need to ensure this program will not have access to those files */
    int i = sysconf (_SC_OPEN_MAX);
    for (; i >= 0; i--)
    {
        close (i);
    }

    /* As an additional security measure, I've read that it's a good idea to
     * reopen file descriptors 0, 1, and 2 to point to /dev/null */
    i = open ("/dev/null", O_RDWR); // stdin
    dup (i);                            // stdout
    dup (i);                            // stderr
};

static void
init_defaults ()
{
    global_options.user = "root";
    global_options.umask = "027";
    global_options.logfile_path = "/home/kyle/Projects/server/test/logfile";
    global_options.errfile_path = "/home/kyle/Projects/server/test/errfile";
    global_options.config_path = "/home/kyle/Projects/server/test/server.conf";
    global_options.script_path = "/home/kyle/Projects/server/test/command.pl";

    global_options.port = "20171";
    
    global_options.ipver = 4;
    global_options.max_instances = 0;


    global_options.interpreter = PERL;

    /* Silly trick to make interpreters use the same storage space
     * as this section of global_options */
    interpreters = &global_options.perl_path;
    interpreters[PERL] = "/usr/bin/perl";
    interpreters[PYTHON] = "/usr/bin/python";
    interpreters[SH] = "/bin/sh";
};

static void
read_env_variables (char** envp)
{

};

int
bind_to_localhost (const char* port, int ipver, int* sockfd)
{
	struct addrinfo hints;
    struct addrinfo* servinfo;
    struct addrinfo* p;
    
    /* Set up the addrinfo struct to
     * -Use ipv4 or ipv6, whichever was specified
     * -Use a stream socket
     * -Use passive mode 
     * */
    memset (&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;//(ipver == 4 ? AF_INET : AF_INET6);
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; 

	int rv;
	if ((rv = getaddrinfo (NULL, port, &hints, &servinfo)) != 0) 
    {
        server_err ("getaddrinfo: %s", gai_strerror (rv));
		return -1;
	}

    /* Find an address to bind to.  Since we just need to bind to ourselves,
     * this should not fail. 
     * */
    int yes = 1;
    int fd;

    char ipstr[INET6_ADDRSTRLEN];
	for (p = servinfo; p != NULL; p = p->ai_next) 
    {
        char* ipver_str;
        void* addr;

        if (p->ai_family == AF_INET)
        {
            struct sockaddr_in* ipv4 = (struct sockaddr_in*) p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver_str = "IPv4";
        }
        else
        {
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*) p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver_str = "IPv6";
        }

        inet_ntop (p->ai_family, addr, ipstr, sizeof ipstr);
        printf ("scanning %s: %s...\n", ipver_str, ipstr);

		if ((fd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
            server_err ("socket");
			continue;
		}

		if (setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof (int)) == -1) {
            server_err ("setsockopt");
            close (fd);
			return -1;
		}
   
        if (bind (fd, p->ai_addr, p->ai_addrlen) == -1) {
			close(fd);
            server_err ("bind");
			continue;
		}

		break;
	}

	if (p == NULL)  
    {
        server_err ("failed to bind");
		return -1;
	}

	freeaddrinfo(servinfo); // all done with this structure

    /* This is the socket we'll return to the caller */
    *sockfd = fd;

    return 0;
}

void
exit_program (int status)
{
    end_logging ();
    close (sockfd);
    close (newfd);
    exit (status);
};

void
exit_child (int status)
{
    end_logging ();
    close (newfd);
    close (sockfd); // sholud be closed already... double Check
    exit (status);
};

