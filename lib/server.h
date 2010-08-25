#ifndef LIB_SERVER_H
#define LIB_SERVER_H

/* This file defines a standard set of library routines that command scripts
 * in perl, python, shell, or even C will need to invoke in order to 
 * effectively communicate with the parent thread and the client.
 * */

/* Initializes the library's static variables with data received from the
 * parent. 
 *  LOGFILE_PATH - the path of our standard log file.  Use this sparingly
 *  ERRFILE_PATH - the path of our standard error file. Use this sparingly
 *  CLIENTFD - the integer file descriptor to our client connection
 *  CHILDREAD - the integer file handle to our read-only pipe to the parent
 *  CHILDWRITE - the integer file handle to our write-only pipe to the parent
 * */
void init (char* logfile_path, char* errfile_path, 
        int clientfd, int childread, int childwrite);

/* Checks whether the given file descriptor is valid. */
int check_fd (int fd);

/* Closes the given file descriptor. */
int close_fd (int fd);




#endif //LIBSERVER_H

