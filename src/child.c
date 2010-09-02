#include "child.h"
#include "bst.h"
#include "logging.h"
#include "debug.h"

/* Includes commands and message headers that can be passed back and forth */
#include "../lib/messaging.h"

#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>





static int nothing_command (struct server_child*, struct message*);
static int send_b_command (struct server_child*, struct message*);
static int send_nb_command (struct server_child*, struct message*);
static int send_wait_command (struct server_child*, struct message*);
static int recv_b_command (struct server_child*, struct message*);
static int recv_nb_command (struct server_child*, struct message*);
static int recv_wait_command (struct server_child*, struct message*);
static int sema_init_command (struct server_child*, struct message*);
static int sema_post_command (struct server_child*, struct message*);
static int sema_wait_command (struct server_child*, struct message*);
static int sema_try_wait_command (struct server_child*, struct message*);
static int lock_init_command (struct server_child*, struct message*);
static int lock_acquire_command (struct server_child*, struct message*);
static int lock_release_command (struct server_child*, struct message*);
static int lock_try_acquire_command (struct server_child*, struct message*);
static int monitor_init_command (struct server_child*, struct message*);
static int monitor_wait_command (struct server_child*, struct message*);
static int monitor_signal_command (struct server_child*, struct message*);
static int monitor_bcast_command (struct server_child*, struct message*);




/* The child index */
static struct child_index index;

/* The command index */
static commandfunc* runcommand[NUM_COMMANDS];

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
    /* Init the child index data structure */
    pthread_mutex_init (&index.lock, NULL);
    bst_init (&index.tree, &child_compare);

    /* Initialize the run commands index */
    runcommand[NOTHING] = &nothing_command;
    runcommand[SEND_B] = &send_b_command;
    runcommand[SEND_NB] = &send_nb_command;
    runcommand[SEND_WAIT] = &send_wait_command;
    runcommand[RECV_B] = &recv_b_command;
    runcommand[RECV_NB] = &recv_nb_command;
    runcommand[RECV_WAIT] = &recv_wait_command;
    runcommand[SEMA_INIT] = &sema_init_command;
    runcommand[SEMA_POST] = &sema_post_command;
    runcommand[SEMA_WAIT] = &sema_wait_command;
    runcommand[SEMA_TRY_WAIT] = &sema_try_wait_command;
    runcommand[LOCK_INIT] = &lock_init_command;
    runcommand[LOCK_ACQIRE] = &lock_acquire_command;
    runcommand[LOCK_RELEASE] = &lock_release_command;
    runcommand[LOCK_TRY_ACQUIRE] = &lock_try_acquire_command;
    runcommand[MONITOR_INIT] = &monitor_init_command;
    runcommand[MONITOR_WAIT] = &monitor_wait_command;
    runcommand[MONITOR_SIGNAL] = &monitor_signal_command;
    runcommand[MONITOR_BCAST] = &monitor_bcast_command;
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
        
    pthread_mutex_lock (&index.lock);
    bst_insert (&index.tree, child);
    pthread_mutex_unlock (&index.lock);
};

struct server_child*
remove_child (int ourid)
{
    ASSERT (ourid > 0);
    
    /* This is used only to compare with and find the correct element */
    struct server_child temp;
    temp.ourid = ourid;

    pthread_mutex_lock (&index.lock);
    struct server_child* result =
        (struct server_child*) bst_delete (&index.tree, &temp);
    pthread_mutex_unlock (&index.lock);

    return result;
};

struct server_child*
get_child (int ourid)
{
    ASSERT (ourid > 0);

    struct server_child temp;
    temp.ourid = ourid;

    pthread_mutex_lock (&index.lock);
    struct server_child* result =
        (struct server_child*) bst_find (&index.tree, &temp);
    pthread_mutex_unlock (&index.lock);

    return result;
};

void
dump_child_index ()
{
    pthread_mutex_lock (&index.lock);
    bst_dump (&index.tree, &child_dump);
    pthread_mutex_unlock (&index.lock);
};


void*
child_comm_thread (void* chld)
{
    ASSERT (chld != NULL);

    struct server_child* child = (struct server_child*) chld;

    server_log ("New thread for child %u, before lock", child->pid);

    /* Once this call returns, the thread is good to go */
    pthread_mutex_lock (&child->init_lock);

    server_log ("%u, beginning loop", child->pid);

    /* Main loop */
    while (true)
    {
        struct message msg;

        /* Wait for the child to send us something */
        read (child->parentread, &msg, sizeof msg);
        enum command cmd = msg.message;   

        /* Now run the child's command or otherwise interpret its message */
        int result = runcommand[cmd](child, &message);
    }

    pthread_mutex_unlock (&child->init_lock);

    pthread_exit ((void*) NULL);
};



/**
 * Handles any response that the child sends us that is not an explicit command,
 * but merely a response that returns data back to the parent.
 * */
static int 
nothing_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);
};

static int 
send_b_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);

    /* Information for sending */
    int child_id = m->id;
    void* data = &m->information;
    size_t sz = m->sz;

    /* Grab information for the specified child */
    struct server_child* sendto = get_child (child_id);
    if (sendto == NULL)
    {
        server_err ("Received a bad child ID from child %d (pid %d) during SEND_B command", 
            me->ourid, me->pid);
        return -1;
    }

    // TODO: need to decide if this data needs to be wrapped in a message to the
    // child or not
    return send (sendto->parent_write, data, sz);
};

static int 
send_nb_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);
};

static int 
send_wait_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);
};

static int 
recv_b_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);
};

static int 
recv_nb_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);
};

static int 
recv_wait_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);
};

static int 
sema_init_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);
};

static int 
sema_post_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);
};

static int 
sema_wait_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);
};

static int 
sema_try_wait_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);
};

static int 
lock_init_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);
};

static int 
lock_acquire_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);
};

static int 
lock_release_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);
};

static int 
lock_try_acquire_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);
};

static int 
monitor_init_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);
};

static int 
monitor_wait_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);
};

static int 
monitor_signal_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);
};

static int 
monitor_bcast_command (struct server_child* me, struct message* m)
{
    ASSERT (m != NULL);
};



