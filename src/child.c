#include "child.h"
#include "logging.h"

#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>

/* Connection with the client */
int clientfd;

/* Communication channel with parent */
int readpipe;
int writepipe;

char**
build_child_argv (char* exe, char* scriptname, int newfd, int childread, int childwrite)
{
    char** argv = (char**) malloc (6 * sizeof (char*));

    char* str_newfd = (char*) malloc (7 * sizeof (char));
    char* str_childread = (char*) malloc (7 * sizeof (char));
    char* str_childwrite = (char*) malloc (7 * sizeof (char));

    if (argv == NULL || str_newfd == NULL || str_childread == NULL ||
            str_childwrite == NULL ||
        0 >= sprintf (str_newfd, "%d", newfd) ||
        0 >= sprintf (str_childread, "%d", childread) ||
        0 >= sprintf (str_childwrite, "%d", childwrite))
    {
        free (argv);
        free (str_newfd);
        free (str_childread);
        free (str_childwrite);

        return NULL;
    }

    /* Parameters need to be passed to the interpreter:
     * 1. Script name
     * 2. newfd
     * 3. childread
     * 4. childwrite
     * ...
     * */
    argv[0] = exe;
    argv[1] = scriptname;
    argv[2] = str_newfd;
    argv[3] = str_childread;
    argv[4] = str_childwrite;
    // ...
    argv[5] = NULL;

    return argv;
};

