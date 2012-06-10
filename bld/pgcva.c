/*   pgcva.c - Convert to format A proggy  */
/*   R. A. Parker   14.02.2012             */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mtax.h"
#include "lay3.h"
#include "lay2do.h"

void pgcva(int mf, int md, int ma)
{
  FIELD * f;            /* context for whole operation */
  Dfmt dp;              /* Start of Dfmt moj */
  uint64 * upd;         /* temporary pointer for d */
  uint64 * upa;         /* temporary pointer for a */
  char * ap;            /* big A-format temporary data */
  char * apm;           /* Real A-format moj */
  uint64 * ast;         /* temporary list of A-starts */
  uint64 alcoves;       /* number of alcoves to make */
  uint64 alc;           /* temporary alcove number */
  uint64 aused;         /* amount of program so far */
    DSPACE ds;
    size_t lena;          /* maximum, then correct, size of A */

    f = (FIELD *) lay3ptr(mf);  /* get the moj pointers */
    dp = (Dfmt) lay3ptr(md);

    upd = (uint64 *) dp;   /* point to them as uint64 */
    if(f[FDEF]!=upd[0])
    {
        printf("Fields do not agree\n");
        exit(31);
    }
    dp+=32;
    alcoves=(upd[2]+f[ALC]-1)/f[ALC];
    lena = LenA(f,upd[1])*alcoves;  /* max size of A-format for matrix */
    ap=malloc(lena);        /*  */
    if(ap==NULL)
    {
        printf("Failed to malloc temporary A-format\n");
        exit(32);
    }
    ast=malloc(8*alcoves);
    if(ast==NULL)
    {
        printf("Failed to malloc temporary start positions\n");
        exit(33);
    }
    aused=0;
    DSSet(f,upd[2],&ds);
    for(alc=0;alc<alcoves;alc++)
    {
        ast[alc]=aused;
        aused+=DtoA(f,&ds,dp,upd[1],alc,ap+aused);
    }
    lay3release(mf);
    apm=lay3mem(ma,32+8*alcoves+aused);
    upa = (uint64 *) apm;   /* point to it as uint64 */
    upa[0]=upd[0];
    upa[1]=upd[1];
    upa[2]=upd[2];
    lay3release(md);
    upa+=4;
    for(alc=0;alc<alcoves;alc++)
        upa[alc]=ast[alc];
    upa+=alcoves;
    memcpy(upa,ap,aused);
    lay3release(ma);
    free(ap);
    free(ast);
}
/* end of pgcva.c  */
