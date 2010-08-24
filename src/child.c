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
build_child_argv ()
{

};

