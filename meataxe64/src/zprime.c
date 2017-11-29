/*
      zprime.c     meataxe-64 Get field size
      ========     J. G. Thackray 26.12.2015
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    EFIL *e;
    uint64 prime;
    uint64 header[5];
    LogCmd(argc,argv);

    if ( (argc < 2) || (argc>3) )
    {
        LogString(80,"usage zprime <m1>");
        exit(14);
    }
    e = ERHdr(argv[1], header);
    (void)e;
    prime = header[1];
    printf("%lu\n", prime);
    return 0;
}
/*  end of zprime.c    */
