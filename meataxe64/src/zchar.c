/*
      zchar.c     meataxe-64 Get field characteristic
      =======     J. G. Thackray 11.12.2016
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "slab.h"
 
int main(int argc,  char **argv)
{
    EFIL *e;
    uint64 header[5];
    uint64 fdef;
    FIELD *f = malloc(FIELDLEN);
    LogCmd(argc,argv);

    if ( (argc < 2) || (argc>3) )
    {
        LogString(80,"usage zchar <m1>");
        exit(14);
    }
    e = ERHdr(argv[1], header);
    (void)e;
    fdef = header[1];
    FieldSet(fdef, f);
    printf("%lu\n", f->charc);
    return 0;
}
