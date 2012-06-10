/*   pgcvd.c - Convert format C to format D proggy  */
/*   R. A. Parker    14.02.2012                     */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mtax.h"
#include "lay3.h"
#include "lay2do.h"

void pgcvd(int mf, int mc, int md)
{
  FIELD * f;            /* context for whole operation */
  Cfmt cp;              /* Start of Cfmt moj */
  uint64 * cpt;         /* c moj as uint64 */
  size_t lend;          /* length of result */
    char * dp;
    uint64 * dpt;
    DSPACE ds;

    f = (FIELD *) lay3ptr(mf);  /* get the moj pointers */
    cp = (Cfmt) lay3ptr(mc);
    cpt=(uint64 *) cp;
    DSSet(f,cpt[2],&ds);
    lend = 32+ds.nob*cpt[1];
    dp=lay3mem(md,lend);
    dpt=(uint64 *) dp;
    dpt[0]=cpt[0];
    dpt[1]=cpt[1];
    dpt[2]=cpt[2];
    CtoD(f,&ds,cp+32,cpt[1],dp+32);
    lay3release(md);
    lay3release(mc);
    lay3release(mf);
}

/* end of pgcvd.c  */
