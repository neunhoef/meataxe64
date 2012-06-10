/*
      zmu.c     MTX64 matrix multiplication
      =====     R. A. Parker   14.02.2012 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mtax.h"
#include "lay2do.h"
#include "lay3.h"
 
int main(int argc,  char **argv)
{
    FILE *f1,*f2,*f3;
    uint64 * tpt;
    uint64 fdef1, nor1, noc1, fdef2, nor2, noc2;
    size_t lenf;
    int fmoj;
    char *fpt;
    FIELD *f;
    int amoj;
    char *apt;
    uint64 * aui;
    int afmoj;
    int bmoj;
    char *bpt;
    uint64 * bui;
    int bfmoj;
    DSPACE ds1,ds2;
    int inp[10];
    int oup[10];
    int res;
    uint64 cauldrons;
    uint64 caul;
    int steermoj;
    int cansmoj;
    int * ansmoj;
    char * d3;
    /* check sanity. */

    if (argc != 4)
    {
        printf("usage zmu m1 m2 ans\n");
        exit(31);
    }

    f1 = RdHdr(argv[1],&fdef1,&nor1,&noc1);
    f2 = RdHdr(argv[2],&fdef2,&nor2,&noc2);
    if( (fdef1!=fdef2) || (noc1!=nor2) )
    {
        printf("Matrices incompatible\n");
        exit(32);
    }

    lay3initialize();

    /* Set the field object */


    fmoj=lay3newmoj();
    lenf=LenHpmi(fdef1,nor1,noc1,noc2);
    fpt=lay3mem(fmoj,lenf);
    f = (FIELD *) fpt;
    res=HpmiSet(fdef1,nor1,noc1,noc2,f);
    if(res==99) printf("99\n");
    lay3release(fmoj);

    /* Set the Dfmt A object */

    DSSet(f,noc1,&ds1);
    amoj=lay3newmoj();
    apt=lay3mem(amoj,ds1.nob*nor1+32);
    aui=(uint64 *) apt;
    aui[0]=fdef1;
    aui[1]=nor1;
    aui[2]=noc1;
    RdMatrix(f1,&ds1,nor1,apt+32);
    Close(f1);
    lay3release(amoj);

    /* Submit the conversion to A-format */

    afmoj=lay3newmoj();
    inp[0]=fmoj;
    inp[1]=amoj;
    inp[2]=0;
    oup[0]=afmoj;
    oup[1]=0;
    lay3submit(CVA,inp,oup);
    lay3deref(amoj);

    /* Set the Dfmt B object */

    DSSet(f,noc2,&ds2);
    bmoj=lay3newmoj();
    bpt=lay3mem(bmoj,ds2.nob*nor2+32);
    bui=(uint64 *) bpt;
    bui[0]=fdef2;
    bui[1]=nor2;
    bui[2]=noc2;
    RdMatrix(f2,&ds2,nor2,bpt+32);
    Close(f2);
    lay3release(bmoj);

    /* Submit the conversion to B-format */

    bfmoj=lay3newmoj();
    inp[0]=fmoj;
    inp[1]=bmoj;
    inp[2]=0;
    oup[0]=bfmoj;
    oup[1]=0;
    lay3submit(CVB,inp,oup);
    lay3deref(bmoj);

    /* now submit the multiplications and conversions */

    cauldrons=(noc2+f[CAUL]-1)/f[CAUL];
    ansmoj = (int *) malloc(cauldrons*sizeof(int));
    if(ansmoj==NULL)
    {
        printf("unable to malloc list of mojes/n");
        exit(37);
    }
    for(caul=0;caul<cauldrons;caul++)
    {
        steermoj=lay3newmoj();
        cansmoj=lay3newmoj();
        ansmoj[caul]=lay3newmoj();
        tpt = (uint64 *) lay3mem(steermoj,8);
        tpt[0]=caul;
        lay3release(steermoj);
        inp[0]=fmoj;
        inp[1]=steermoj;
        inp[2]=afmoj;
        inp[3]=bfmoj;
        inp[4]=0;
        oup[0]=cansmoj;
        oup[1]=0;
        lay3submit(MUL,inp,oup);
        lay3deref(steermoj); 
        inp[0]=fmoj;
        inp[1]=cansmoj;
        inp[2]=0;
        oup[0]=ansmoj[caul];
        oup[1]=0;
        lay3submit(CVD,inp,oup);
        lay3deref(cansmoj);  
    }
    lay3deref(afmoj);
    lay3deref(bfmoj);
/* all submitted - now wait for all work to be done  */
    for(caul=0;caul<cauldrons;caul++)
        lay3waitmoj(ansmoj[caul]);
/* all read, now just work out the answer and write it out  */
    f3=WrHdr(argv[3],fdef1,nor1,noc2);
    d3=malloc(nor1*ds2.nob);
    if(d3==NULL)
    {
        printf("Unable to malloc answer memory\n");
        exit(33);
    }
    memset(d3,0,nor1*ds2.nob);

    for(caul=0;caul<cauldrons;caul++)
    {
        apt=lay3ptr(ansmoj[caul]);
        if(caul+1<cauldrons)
            DSSet(f,f[CAUL],&ds1);
        else
            DSSet(f,noc2-(cauldrons-1)*f[CAUL],&ds1);
        apt+=32;

        DPaste(&ds1,apt,nor1,caul*f[CAUL],&ds2,d3);

    }
    WrMatrix(f3,&ds2,nor1,d3);
    return 0;
}      /******  end of zmu.c    ******/
