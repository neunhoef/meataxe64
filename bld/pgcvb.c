/*   pgcvb.c - Convert to format B proggy  */
/*   R. A. Parker    14.02.2012            */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mtax.h"
#include "lay3.h"
#include "lay2do.h"

void pgcvb(int mf, int md, int mb)
{
  FIELD * f;            /* context for whole operation */
  Dfmt dp;              /* Start of Dfmt moj */
  uint64 * upd;         /* temporary pointer for d */
  char * bpm;           /* Real B-format moj */
  uint64 * upb;         /* temporary pointer for b */
  size_t lenb;          /* size of B  */
    DSPACE ds;

    f = (FIELD *) lay3ptr(mf);  /* get the moj pointers */
    dp = (Dfmt) lay3ptr(md);

    upd = (uint64 *) dp;   /* point to them as uint64 */
    if(f[FDEF]!=upd[0])
    {
        printf("Fields do not agree\n");
        exit(31);
    }
    dp+=32;
    lenb = LenB(f,upd[1],upd[2]);
    bpm=lay3mem(mb,32+lenb);
    upb = (uint64 *) bpm;   /* point to it as uint64 */
    upb[0]=upd[0];
    upb[1]=upd[1];
    upb[2]=upd[2];
    DSSet(f,upd[2],&ds);
    DtoB(f,&ds,dp,upb[1],bpm+32);
    lay3release(mb);
    lay3release(mf);
    lay3release(md);
}

/* end of pgcvb.c  */
