#ifndef LIB_SERVER_H
#define LIB_SERVER_H

/*
 * This library serves as the standard interface for custom programs to 
 * interact with the server.  Modules compiled for other languages such as
 * perl and python will be build off this library, and functions will 
 * resemble the functions below as closely as possible so as to avoid
 * confusion.  
 *
 * There are several sections below, with each section containing functions
 * relevant to different types of operations the server script may wish to
 * perform.  Many functions are included here simply because C is required
 * to perform these functions, as they rely on having access to system calls
 * dealing with single unix integer file handles.  
 *
 * The starting point of the script is the init function, where data will be
 * passed from the client library (a perl, python, shell, etc script).  
 * This data should have been obtained directly from the process's argv, 
 * however, and should have only undergone enough manipulation to convert
 * char* values to integer values.
 *
 * The various types of operations are as follows:
 *
 *  - Client Communication: functions to handle communication with the client.
 *          These functions include mainly blocking and non-blocking
 *          send/recv operations.
 *
 *  - Inter-Process Communication: functions to handle talking to other
 *          server processes.  This may be an essential part of certain
 *          server applications, so a full range of functions are made 
 *          available, including blocking and non-blocking send/recv
 *          operations, synchronization constructs such as mutexes, 
 *          semaphores, monitors, and spinlocks, and the ability to poll
 *          other processes for status updates and trigger events.
 *
 *  - Miscellaneous: All other functions which don't fall under any particular
 *          category.  The functions are listed below, but some of them 
 *          include logging to the global logs, logging to the local logs, 
 *          requesting process list, obtaining the connection status, and
 *          setting events for status changes.
 * */


/* Holds an ip address.  ip_ver specifies 4 or 6, and each octet is stored in
 * o1 - o4.  For an IPv4 address like 192.168.1.126, you would have:
 *          ip_ver = 4
 *          o1 = 192
 *          o2 = 168
 *          o3 = 1
 *          o4 = 126
 * And for an IPv6 address like 2001:0db8:c9d2:aee5:73e3:934a:a5ae:9551,
 *          ip_ver = 6
 *          o1 = (2001 << 4) | 0db8
 *          o2 = (c9d2 << 4) | aee5
 *          o3 = (73e3 << 4) | 934a
 *          o4 = (a5ae << 4) | 9551
 * */
typedef struct
{
    int ip_ver;
    unsigned o1;
    unsigned o2;
    unsigned o3;
    unsigned o4;
} ip_addr_t;

/* Indicates our connection status with the client */

typedef enum conn_state
{
    CONN_CONNECTED = 0x1,
    CONN_DISCONNECTED = 0x2,
    CONN_SENDING = 0x4,     // performing a non-blocking send
    CONN_RECVING = 0x8,     // performing a non-blocking recv
    CONN_CLOSING = 0x10,    // requesting to close with client
    CONN_TERMING = 0x11,    // requesting to terminate
    CONN_RESET = 0x12,      // requesting to reset
    CONN_WAITSEND = 0x14,   // performed a blocking send
    CONN_WAITRECV = 0x18    // performed a blocking recv
} conn_state_t;

typedef struct
{
    int port;
    ip_addr_t ip;
    enum conn_state state;
} conn_status_t;

struct proc_info
{
    int proc_id;            // server ID of the process, NOT the system pid
    ip_addr_t client_ip;    // The client's ip address
    conn_state_t conn_state;// the state of this process's connection
    struct proc_info* next;      // used to access a list of all processes
};
typedef struct proc_info proc_info_t;

/* Initializes the library's static variables with data received from the
 * parent. 
 *  LOGFILE_PATH - the path of our standard log file.  Use this sparingly
 *  ERRFILE_PATH - the path of our standard error file. Use this sparingly
 *  CLIENTFD - the integer file descriptor to our client connection
 *  CHILDREAD - the integer file handle to our read-only pipe to the parent
 *  CHILDWRITE - the integer file handle to our write-only pipe to the parent
 *  IPADDR - the ip address of the client
 *  IPVER - the ip address version: 4 or 6
 *  port - the port the child is connected to 
 * */
void init (char* logfile_path, char* errfile_path, 
        int clientfd, int childread, int childwrite,
        char* ipaddr, int ipver, int port);

/* Log a message to the custom log file */
void log_msg (char* format, ...);

/* Log an error to the custom log file */
void err_msg (char* format, ...);


/* *
 * *                    [ Client Communication ] 
 * */

/* The functions below describe operations for communicating with the client.
 * Many of these functions return -1 on error.  The specific error message
 * can be found in the static variable serverr. */

/**
 * Defines the type for a message handle.  These are used only for 
 * non-blocking, or asynchronous communications.  Any non-blocking operation
 * will return a message handle, which is then used later to wait for that
 * request to complete. */
typedef int message_handle_t;

/**
 * Send a message to the client stored at DATA and SZ bytes long.  This
 * function blocks until the message is sent fully or until an error occurs.
 * Returns the number of bytes sent on success, -1 on error. */
int client_send_b (void* data, size_t sz);
/**
 * Send a message to the client stored at DATA and SZ bytes long.  This
 * function does not block, but instead returns a handle.  Call CLIENT_WAIT
 * with the given handle to wait on this communication. */
message_handle_t client_send_nb (void* data, size_t sz);
/**
 * Waits for a send operation to complete.  MH is a message handle which
 * should have been obtained by performing a nonblocking send operation
 * using CLIENT_SEND_NB.  This function returns the number of bytes sent. */
int client_wait_send (message_handle_t mh);

/**
 * Receive a message from the client.  The message will be stored at DATA, 
 * and will be no more than SZ bytes long.  This function blocks until the
 * message is fully received or until an error occurs.  Returns the number
 * of bytes received on success, -1 on error. */
int client_recv_b (void* data, size_t sz);
/**
 * Receive a message from the client, but do not block.  The message will be
 * stored at DATA and be no larger than SZ bytes.  It is NOT SAFE to access
 * this data until CLIENT_WAIT_RECV has been called on the message handle. */
message_handle_t client_recv_nb (void* data, size_t sz);
/**
 * Waits for a recv operation to complete.  MH is a message handle which
 * should have been obtained by performing a nonblocking recv operation
 * using CLIENT_RECV_NB.  This function returns the number of bytes received*/
int client_wait_recv (message_handle_t mh);

/**
 * Closes the connection with the client by sending MSG, of size MSG_SZ.  
 * The connection will be closed when the client returns a response, which
 * will be stored in RESPONSE and be no larger than RESP_SZ.  Note that this
 * function blocks.  The data sent to the client and received from the client
 * is not established by this library; it should be part of the protocol by
 * which the server and client communicate.  Returns the number of bytes
 * received on success, or -1 if any part of the communication failed. */
int close_connection (void* msg, int msg_sz, void* response, int resp_sz);
/**
 * Terminates the connection, sending MSG to the client.  This function will
 * block, but times out after TIMEOUT seconds, which is defined as a static
 * variable.  Returns -1 if an error or timeout occurred, or the number of
 * bytes sent on success. */
int terminate_connection (void* msg, int msg_sz);
/**
 * Attempts to reset the connection by disconnecting then reconnecting with
 * the client.  The server sends MSG, and once reconnected, will receive a
 * response in RESPONSE.  Returns the number of bytes received if 
 * the connection was successfully reestablished, -1 on failure. */
int reset_connection (void* msg, int msg_sz, void* response, int resp_sz);

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



#endif //LIBSERVER_H

