#include "bst.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>

static int compare (const void* a, const void* b, const void* AUX);
static void dump (const void* a);

/* Emulate main.c's variable RUN here.  We can now run tests on this 
 * variable */
int run = 1;

int 
main (int argc, char** argv)
{
    struct bst_tree a;
    bst_init (&a, &compare);

    int t1 = 1;
    int t2 = 2;
    int t3 = 3;
    int t4 = 4;
    int t5 = 5;
    int t6 = 6;
    int t7 = 7;
    int t8 = 8;
    int t9 = 9;
    int t0 = 0;

    bst_insert (&a, &t1);

    bst_dump (&a, &dump);

    bst_insert (&a, &t2);

    bst_dump (&a, &dump);

    bst_insert (&a, &t3);

    bst_dump (&a, &dump);

    bst_insert (&a, &t4);

    bst_dump (&a, &dump);

    bst_insert (&a, &t5);

    bst_dump (&a, &dump);

    bst_insert (&a, &t6);

    bst_dump (&a, &dump);

    bst_insert (&a, &t7);

    bst_dump (&a, &dump);

    bst_insert (&a, &t8);

    bst_dump (&a, &dump);

    bst_insert (&a, &t9);

    bst_dump (&a, &dump);

    bst_insert (&a, &t0);

    bst_dump (&a, &dump);


};

static int 
compare (const void* a, const void* b, const void* AUX)
{
    return *((int*) a) - *((int*) b);
};

static void
dump (const void* a)
{
    printf ("%d", *((int*) a));
};

