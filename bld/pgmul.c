/*   pgmul.c - multiply proggy  */
/*   R. A. Parker   14.02.2012  */

#include <stdio.h>
#include <stdlib.h>
#include "mtax.h"
#include "lay3.h"
#include "lay2do.h"

void pgmul(int fmoj, int smoj, int amoj, int bmoj, int cmoj)
{
    FIELD * f;
    uint64 * s;
    uint64 cauld;
    uint64 alcove;
    uint64 alcoves;
    char * ap;
    uint64 * apt;
    uint64 bstride;
    char * bp;
    uint64 * bpt;
    char * cp;
    uint64 * cpt;
    size_t csiz;
    size_t lenbrick;
    Brick bk;
    char * aprog;
    uint64 nocout;

    f=(FIELD *) lay3ptr(fmoj);
    s=(uint64 *) lay3ptr(smoj);

    cauld=*s;

    lay3release(smoj);

    ap=lay3ptr(amoj);
    apt=(uint64 *) ap;
    bp=lay3ptr(bmoj);
    bpt=(uint64 *) bp;
    bp+=32;
    csiz=apt[1]*f[BCAUL]+32;
    nocout=bpt[2]-cauld*f[CAUL];
    if(nocout>f[CAUL]) nocout=f[CAUL];
    cp=lay3mem(cmoj,csiz);
    cpt=(uint64 *) cp;
    cpt[0]=apt[0];
    cpt[1]=apt[1];
    cpt[2]=nocout;

    cp+=32;

    ZerC(f,cp,1,apt[1]);

    alcoves=(apt[2]+f[ALC]-1)/f[ALC];
    apt+=4;
    ap+=32+8*alcoves;
    lenbrick=LenBrick(f);
    bk=cachemalloc(lenbrick);
    bstride = f[BCAUL]*((bpt[2]+f[CAUL]-1)/f[CAUL]);
    for(alcove=0;alcove<alcoves;alcove++)
    {
        aprog=ap+apt[alcove];
        BrickPop(f,bp,cauld,bstride,bk);
        BrickMad(f,aprog,bk,cp);
        bp+=f[ALC]*bstride;
    }
    lay3release(fmoj);
    lay3release(amoj);
    lay3release(bmoj);
    lay3release(cmoj);
}
/* end of pgmul.c  */
