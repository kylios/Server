#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "logging.h"

static FILE* logfile;
static FILE* errfile;

static time_t t;
static struct tm* local;

static char* days[7] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static char* months[12] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

/* PID to write to the log */
static pid_t pid = 0;

static void print_time (FILE* fd);
static void print_pid (FILE* fd);

/* Keep our file accesses synchronized */
static sem_t* logsem = NULL;
static sem_t* errsem = NULL;

int 
init_logging (char* logfile_path, char* errfile_path)
{
    logfile = fopen (logfile_path, "a");
    errfile = fopen (errfile_path, "a");

    int try = 0;
    do {
        if (!logsem || logsem == SEM_FAILED)
        {
            logsem = sem_open ("/server_logsem", O_CREAT);
        }
        if (!errsem || errsem == SEM_FAILED)
        {
            errsem = sem_open ("/server_errsem", O_CREAT);
        }
        try++;
    } while ((logsem == SEM_FAILED || errsem == SEM_FAILED) &&
            try < 10);

    if (logsem == SEM_FAILED || errsem == SEM_FAILED)
    {
        return -1;
    }

    server_log ("\n--------------------------------------------------\n");
    return 0;
};

void
set_log_child (pid_t p)
{
    pid = p;
};

void
server_log (const char* format, ...)
{
    va_list args;

    sem_wait (logsem);

    /* Since another process may have written to this file, we need to
     * make sure the current pos is at the end */
    fseek (logfile, 0, SEEK_END);

    print_time (logfile);
    print_pid (logfile);

    va_start (args, format);
    vfprintf (logfile, format, args);
    va_end (args);

    fprintf (logfile, "\n");

    sem_post (logsem);
};

void 
server_err (const char* format, ...)
{
    va_list args;

    sem_wait (errsem);

    fseek (errfile, 0, SEEK_END);

    print_time (errfile);
    print_pid (errfile);

    va_start (args, format);
    vfprintf (errfile, format, args);
    va_end (args);

    fprintf (errfile, "\n");

    sem_post (errsem);
};

void
print_err (int err)
{
//    char buf[256];
//    strerror (err, buf, sizeof buf);    
//    server_err ("%s", buf);
    server_err ("%s", strerror (err));
};

void
end_logging ()
{
    fclose (logfile);
    fclose (errfile);

    sem_close (logsem);
    sem_close (errsem);
};

void
print_time (FILE* fd)
{
    struct tm* l;

    t = time (NULL);
    l = local = localtime (&t);
 
    // format the time like [YYYY/MM/DD HH:MM:SS]
    const char* format = "[%s %s %d, %d %02d:%02d:%02d] ";
    const char* day = days[l->tm_wday];
    const char* month = months[l->tm_mon];
    fprintf (fd, format, 
            day, month, l->tm_mday, (1900 + l->tm_year), 
            l->tm_hour, l->tm_min, l->tm_sec);
};

void
print_pid (FILE* fd)
{
    if (pid == 0)
    {
        fprintf (fd, "server: ");
    }
    else
    {
        fprintf (fd, "%u: ", pid);
    }
};

