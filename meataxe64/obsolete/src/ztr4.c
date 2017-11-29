/*
      ztr4.c     meataxe-64 big (IV) transpose [unfinished]
      ======     R. A. Parker 4.5.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include "field.h"
#include "io.h"
#include "tuning.h"
#include "tfarm.h"
#include "funs.h"

int main(int argc,  char **argv)
{
    uint64 fdef,nor,noc;
    FIELD * f;
    uint64 HDR[5];
    int res;
    int mojin,mojout,rchops,cchops,fmoj;

    LogCmd(argc,argv);
    EPeek(argv[1],HDR);
    fdef=HDR[1];
    nor=HDR[2];
    noc=HDR[3];
    fmoj=0;
    mojin=1;
    rchops=9;
    cchops=9;
    mojout=mojin+rchops*cchops;
    TfInitialize(mojout+rchops*cchops,0,THREADS+1,1);
    TfGo();
    f=TfAllocate(0,FIELDLEN);
    res=FieldSet(fdef,f,0);
    (void)res;
    if (argc != 3)
    {
        LogString(80,"usage ztr m1 m1tr");
        exit(21);
    }
    fch(f,argv[1],4,4);
    for(i=0;i<frchops;i++)
    {
        for(j=0;j<fcchops;j++)
        {
// make the filenames
// check mojes can be re-used
// field should be a moj
            ftr(f,fin,fout,rchops,cchops, mojin, mojout);
        }
    }
    fas(f,nor,noc,argv[1],4,4);
    return 0;
}

/*  end of ztr4.c    */
