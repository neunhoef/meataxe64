/*   hpmi.c - MTAX-64 High Performance Meataxe mod 2 only   */
/*   Targetted towards version 1.00                         */
/*   R. A. Parker 29.02.2012                                */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mtax.h"

size_t LenHpmi(uint64 fdef, uint64 rowa, uint64 cola, uint64 colb)
{
  (void)fdef;
  (void)rowa;
  (void)colb;
  (void)cola;
    return 256;    /* just the basic 32 uint64 words  */
}

size_t LenBrick(FIELD * f)
{
   return 256+f[BCAUL]*f[FSL]*(1<<f[FGC]);
}

int HpmiSet(uint64 fdef, uint64 rowa, uint64 cola, 
                                  uint64 colb, FIELD * f)
{
    int grlev, s8;
  (void)rowa;
  (void)colb;
  (void)cola;
    if(fdef!=2)
    {
        printf("field2 compiled in, so it only works mod 2\n");
        exit(7);
    }
/* Decide what grease level and slice count to use  */
    grlev=8;    /* need something a bit more sophisticated later */
    s8=2;       /* lots of 8 slices  */
    
    f[FDEF] =2;     /* field definition is 2 */
    f[CHARC]=2;     /* characteristic is 2 */
    f[POW]  =1;     /* power is 1 (not an extension field) */
    f[CONP] =1;     /* Conway polynomial is X = 1 */
    f[ALC]  =grlev*s8*8;   /* columns in an Alcove of A */
    f[CAUL] =8*CACHELINE;       /* columns in a cauldron */
    f[BCAUL]   =CACHELINE;         /* bytes for a row of a cauldron. */
    f[FGC]=grlev;             /* grease level */
    f[FSL]=8*s8;
    return 0;
}

void BrickPop(FIELD * f, Bfmt b, uint64 cauldron, 
                                uint64 bstride, Brick bk)
{
    unsigned int i, j, k;
    int pwr2;
    unsigned int slice;
    uint64 inclor;                /* inclusive or of all words */
    uint64 * BrickType;           /* uint64 pointer to brick */
    uint64 * SlicePtr;            /* pointer to zero of slice */
    uint64 *gptr, *g1,*g2;        /* temporary pointers to grease row */
    uint64 * BfmtPtr;             /* pointer to input data */
    uint64 bwords;                /* bstride in units of uint64 */

    inclor=0;
    BrickType = (uint64 *) bk;
    BfmtPtr = (uint64 *)  b;      /* start of matrix b */
    BfmtPtr += cauldron*(f[BCAUL]/8);       /* correct place in first row */
    bwords=bstride/8;

/* first get the rows from Bfmt into positions 2^i of each  */
/* of the 16 slices of the brick                            */
/* keeping "inclor" as their inclusive or                   */

    SlicePtr=BrickType+32;           /* first slice */
    for(slice=0;slice<f[FSL];slice++)
    {
      for(i=0;i<(f[BCAUL]/8);i++) SlicePtr[i]=0;   /* clear out zero row! */
        pwr2=1;
        for(i=0;i<f[FGC];i++)           /* get the data rows */
        {
          gptr=SlicePtr + (pwr2*(f[BCAUL]/8)); /* where does it go in brick */
          for(j=0;j<(f[BCAUL]/8);j++)          /* move the words b -> brick */
            {
                inclor  |=BfmtPtr[j];
                *(gptr++)=BfmtPtr[j];
            }
          BfmtPtr+=bwords;          /* to middle of next row */
          pwr2<<=1;                 /* next power of 2  */
        }
        SlicePtr+=(1<<f[FGC])*(f[BCAUL]/8);  /* ready for next slice */
    }

/* 0, 1, 2, 4, 8, 16, 32, 64, 128 correct so far            */

/* if inclor==0, just set the BrickType to 0 and exit       */
/* otherwise set Bricktype to be 1                          */
    if(inclor==0)
    {
        BrickType[0]=0;
        return;
    }
    BrickType[0]=1;
    BrickType[1]=60;
    if(CACHELINE==64) BrickType[1]=3;
    if(CACHELINE==128) BrickType[1]=4;
    if(BrickType[1]==60)
    {
        printf("CACHELINE not set to 64 or 128\n");
        exit(5);
    }
    BrickType[2]=f[FSL]/8;        /* s8 */
    BrickType[3]=f[FGC];      /* grease level */

/* then make all the other rows of the grease table         */

    SlicePtr=BrickType+32;           /* back to first slice */
    for(slice=0;slice<f[FSL];slice++)
    {
      for(i=3;i<(1U<<f[FGC]);i++)            /* do all 256 rows now */
        {
          gptr=SlicePtr + (i*(f[BCAUL]/8));   /* where does it go in brick */
/* now find a j (if there is one) with both j and j^i less than i */
            j=1;
            while( (j^i) > i) j<<=1;  /* termination guaranteed! */
            if(j==i) continue;        /* power of 2 already in place   */
            g1 = SlicePtr + (j*(f[BCAUL]/8));
            g2 = SlicePtr + ((i^j)*(f[BCAUL]/8));        
            for(k=0;k<(f[BCAUL]/8);k++)         /* make all words of row */
                *(gptr++)=*(g1++) ^ *(g2++); 
        }
      SlicePtr+=(1<<f[FGC])*(f[BCAUL]/8);  /* ready for next slice */
    }
}

size_t LenA(FIELD * f, uint64 nor)
{
    return (1+(1+f[FSL])*nor);
}

size_t DtoA(FIELD * f, DSPACE * ds, Dfmt d, uint64 nor,
                                    uint64 alcove, Afmt a)
{
    unsigned int i, j, k, topb;
    uint8 *ap;          /* start of Afmt as a uint8 */
    size_t lena;        /* displacement to current byte of Afmt */
    uint8 *ptd;         /* pointer into Dfmt input */
    uint64 incor;       /* inclusive-or of Dfmt to check for zero */
    uint64 byt;
    uint64 bong;        /* to contain the whole slice's data */

    ap=(uint8 *) a;     /* make the program array */
    lena=1;             /* program counter "zero" so far */
    ap[0]=0;            /* no rows to skip yet */

    ptd=(uint8 *) d;    /* start of Dfmt input */
    ptd+=alcove*f[FGC]*f[FSL]/8;     /* skip to starting point */
/* compute topb as the number of bytes we can grab from input  */
    topb=ds->nob-(alcove*f[FGC]*f[FSL]/8);
    if(topb>f[ALC]/8) topb=f[ALC]/8;

    for(i=0;i<nor;i++)           /* for every row of the alcove */
    {
      incor=0;                 /* no set bits found yet */
        for(j=0;j<f[FSL]/8;j++)
        {
            bong=0;
            for(k=0;k<f[FGC];k++)
            {
                byt=0;
                if(j*f[FGC]+k < topb) byt=ptd[j*f[FGC]+k];
                bong|=byt<<(8*k);
            }
            incor|=bong;
            for(k=0;k<8;k++)
                ap[lena+8*j+k]=(bong>>(f[FGC]*k))&((1<<f[FGC])-1);
        }
        if( (incor==0) && (ap[lena-1]<254) )
          {                             /* if we can just skip the row */
            ap[lena-1]++;             /* do so */
            ptd+=ds->nob;
            continue;
        }
        lena+=1+f[FSL];           /* if we can't, output a row of Afmt */
        ap[lena-1]=0;                /* and set no skip yet */
        ptd+=ds->nob;
    }
    ap[lena-1]=255;                  /* put on the terminator */
    return lena;                 /* and return the length of the program */
}

size_t LenB(FIELD * f, uint64 nor, uint64 noc)
{
    uint64 alcs, cauls;
    alcs =(f[ALC]+nor-1)/f[ALC];
    cauls=(f[CAUL]+noc-1)/f[CAUL];
    return (alcs*cauls*f[ALC]*f[BCAUL]);
}

void DtoB(FIELD * f, DSPACE * ds, Dfmt d, uint64 nor, Bfmt b)
{
    uint64 i,bstride,lenset;
    bstride=((ds->nob+f[BCAUL]-1)/f[BCAUL])*f[BCAUL];
    lenset=bstride-ds->nob;
    for(i=0;i<nor;i++)
    {
        memcpy(b,d,ds->nob);
        if(lenset!=0) memset(b+ds->nob,0,lenset);
        b+=bstride;
        d+=ds->nob;
    }
}

void CtoD(FIELD * f, DSPACE * ds, Cfmt c,
                               uint64 nor, Dfmt d)
{
    char * cp;
    char * dp;
    unsigned int bytes;
    uint64 i;

    cp=c;
    dp=d;
    bytes=ds->nob;
    if(bytes>f[BCAUL]) bytes=f[BCAUL];
    for(i=0;i<nor;i++)
    {
        memcpy(dp,cp,bytes);
        cp+=f[BCAUL];
        dp+=ds->nob;
    }
}

void ZerC(FIELD * f, Cfmt c, uint64 cauldrons, uint64 nor)
{
    memset(c,0,cauldrons*nor*f[BCAUL]);
}

/* end of hpmi.c  */
