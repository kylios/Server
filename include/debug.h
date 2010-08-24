#ifndef DEBUG_H
#define DEBUG_H

#include "type.h"
//#include "common.h"
#include <stdio.h>
#include <stdlib.h>

void debug_backtrace (void);
void hexdump (void* start, uint32 sz);

#define ASSERT(expression)  \
    if (!(expression))  {   \
        printf ("!!!Failed assertion: `%s' in %s: %d in %s \n\n", \
            #expression, __FILE__, __LINE__, __func__);    \
        debug_backtrace (); \
        exit (EXIT_FAILURE); \
    }

#define DEBUG_MARK  \
    printf ("DEBUG %s: %d (`%s')\n", __FILE__, __LINE__, __func__);

#define PANIC(MSG)  \
    printf ("!!!PANIC: %s \n", MSG); \
    exit (EXIT_FAILURE);

#define NOT_REACHED \
    PANIC("Code section should not have been reached");


#endif // DEBUG_H

