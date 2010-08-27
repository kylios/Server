#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// sockets
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>


#include "server.h"
#include "../include/debug.h"



/* Need to keep these variables in the static context so we can access them
 * whenever we need. */
static int clientfd;
static int childread;
static int childwrite;

/* Client info */
static ip_addr_t ipaddr;
static int port;

/* Checks whether the given file descriptor is valid. */
static int check_fd (int fd);

/* Closes the given file descriptor. */
static void close_fd (int fd);

/* Convert an ip address from a string to type IP_ADDR_T.
 * It is assumed that an ip address of type 4 will have the format XX.XX.XX.XX
 * while an ip address of type 6 will have the type 
 * XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX. */
void convert_ip_address (ip_addr_t* ipaddr, char* str_ipaddr, int ipver);

/* Custom logfile handles */
static FILE* logfile;
static FILE* errfile;


void
init (char* logfile_path, char* errfile_path, 
        int p_clientfd, int p_childread, int p_childwrite,
        char* ipaddr, int ipver, int port)
{
    ASSERT (check_fd (p_clientfd));
    ASSERT (check_fd (p_childread));
    ASSERT (check_fd (p_childwrite));

    logfile = fopen (logfile_path, "a");
    errfile = fopen (errfile_path, "a");

    clientfd = p_clientfd;
    childread = p_childread;
    childwrite = p_childwrite;
};

void
log_msg (char* format, ...)
{
    va_list args;
    va_start (args, format);
    vfprintf (logfile, format, args);
    va_end (args);
};

void
err_msg (char* format, ...)
{
    va_list args;
    va_start (args, format);
    vfprintf (errfile, format, args);
    va_end (args);
};

/* * 
 * *                    [ Client Communications ] 
 * */

/* Defines the number of seconds before a timeout occurs during any client 
 * communication.  */
static int _timeout;
#define TIMEOUT _timeout

/* Defines specific errors which could occur during server-client 
 * communications. */
static int serverr;

static inline void
set_serverr (int val)
{
    if (-1 == val)  serverr = errno;
};

int 
client_send_b (void* data, size_t sz)
{
    int ret = send (clientfd, data, sz, 0);
    set_serverr (ret);
    return ret;
};

message_handle_t
client_send_nb (void* data, size_t sz)
{
    return -1;    
};

int
client_wait_send (message_handle_t mh)
{
    return -1;
};

int 
client_recv_b (void* data, size_t sz)
{
    int ret = recv (clientfd, data, sz, 0);    
    set_serverr (ret);
    return ret;
};

message_handle_t
client_recv_nb (void* data, size_t sz)
{
    return -1;    
};

int 
client_wait_recv (message_handle_t mh)
{
    return -1;
};

int 
close_connection (void* msg, int msg_sz, void* response, int resp_sz);

int 
terminate_connection (void* msg, int msg_sz);

int 
reset_connection (void* msg, int msg_sz, void* response, int resp_sz);

/* [ IPC ] */
int proc_send_b (int procid, void* data, size_t sz);
int proc_send_nb (int procid, void* data, size_t sz);
int proc_recv_b (int procid, void* data, size_t sz);
int proc_recv_nb (int procid, void* data, size_t sz);
void log_message (char* format, ...);
void log_error (char* format, ...);

/* [ Miscellaneous Functions ] */
conn_status_t* get_connection_status (conn_status_t* status);
int get_procid ();
ip_addr_t get_client_ip ();




int
check_fd (int fd)
{
    return (-1 != fcntl (fd, F_GETFL));
};

void
close_fd (int fd)
{
    close (fd);
};

void 
convert_ip_address (ip_addr_t* ipaddr, char* str_ipaddr, int ipver)
{
    char buf[40];   // Copy the ip address here since it gets modified
    strncpy (buf, str_ipaddr, 39);
    buf[39] = '\0';
    memset (ipaddr, 0, sizeof (*ipaddr));
    unsigned* i = &ipaddr->o1;
    int count = 0;
    if (ipver == 4)
    {
        char* pos, *tok;
        for (   tok = strtok_r (buf, ".", &pos);
                tok && count++ < 4;
                tok = strtok_r (NULL, ".", &pos))
        {
            *(i++) = (unsigned) atoi (tok);        
        }
        ipaddr->ip_ver = ipver;
    }
    else if (ipver == 6)
    {
        char* pos, *tok;
        for (   tok = strtok_r (buf, ":.", &pos);
                tok && count++ < 4;
                tok = strtok_r (NULL, ":.", &pos))
        {
            unsigned val1 = 0, val2 = 0;
            sscanf (tok, "%x", &val1);
            if (tok != NULL)
            {
                tok = strtok_r (NULL, ":.", &pos);
                sscanf (tok, "%x", &val2);
            }
            *(i++) = (val1 << (sizeof (unsigned) * 4)) |  val2;
        }
        ipaddr->ip_ver = ipver;
    }
};
