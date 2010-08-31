#include "child.h"
#include "bst.h"
#include "logging.h"
#include "debug.h"

#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
//
///* Connection with the client */
//int clientfd;
//
///* Communication channel with parent */
//int readpipe;
//int writepipe;




/* The child index */
static struct child_index index;

char**
build_child_argv (const char* exe, char* scriptname, int newfd, 
        int childread, int childwrite)
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


void
init_child_index ()
{
    bst_init (&index.tree, &child_compare);
};

int
child_compare (const void* a, const void* b, const void* AUX)
{
    ASSERT (a != NULL);
    ASSERT (b != NULL);

    const struct server_child* child1 = (const struct server_child*) a;
    const struct server_child* child2 = (const struct server_child*) b;

    return child1->ourid - child2->ourid;
};

void 
child_dump (const void* child_elem)
{
    ASSERT (child_elem != NULL);    

    const struct server_child* child = (const struct server_child*) child_elem;

    printf ("[ ID: %d ]", child->ourid);
};

void
add_child (struct server_child* child)
{
    ASSERT (child != NULL);
        
    bst_insert (&index.tree, child);
};

struct server_child*
remove_child (int ourid)
{
    ASSERT (ourid > 0);
    
    /* This is used only to compare with and find the correct element */
    struct server_child temp;
    temp.ourid = ourid;

    return (struct server_child*) bst_delete (&index.tree, &temp);
};

struct server_child*
get_child (int ourid)
{
    ASSERT (ourid > 0);

    struct server_child temp;
    temp.ourid = ourid;

    return (struct server_child*) bst_find (&index.tree, &temp);
};

void
dump_child_index ()
{
    bst_dump (&index.tree, &child_dump);
};


void*
child_comm_thread (void* chld)
{
    ASSERT (chld != NULL);

    struct server_child* child = (struct server_child*) chld;

    /* Once this call returns, the thread is good to go */
    pthread_mutex_lock (&child->init_lock);

    /* Main loop */
    while (true)
    {

    }

    pthread_mutex_unlock (&child->init_lock);

    pthread_exit ((void*) NULL);
};



