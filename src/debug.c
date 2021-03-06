#include "debug.h"
#include "type.h"
#include <stdio.h>
//#include "lib/kernel/framebuf.h"

   
/**
 * This function was taken from the Pintos operating system.
 * See:
 *  http://courses.cs.vt.edu/~cs3204/fall2009/pintos/doc/pintos_1.html#SEC12
 * for more information on Pintos's license.
 * */
void
debug_backtrace (void)
{
  void **frame;

  printf ("Call stack: %p \n", __builtin_return_address (0));
  for (frame = __builtin_frame_address (1);
       (void*) frame >= (void*) 0x1000 && 
       frame[0] != NULL;
       frame = frame[0])
  {
      printf (" %p", frame[1]);
  }
  printf (".\n");
};

void
hexdump (void* _start, uint32 sz)
{
    uint8* start = _start;
    uint32 sigbyte = 0;
    unsigned i;

    while (sigbyte < sz)
    {
        printf ("%x: ", sigbyte);
        for (i = 0; i < 16; i++)
        {
            uint8 b = *start++;
            printf ("%x ", b);
        }
        printf ("\n");
        sigbyte += 16;
    }
};
