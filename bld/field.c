/*
         field2.c   -   Meataxe-64 Mod 2 only Field Routines
         ========       R. A. Parker    14.02.2012
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mtax.h"

size_t LenField(uint64 fdef)
{
  (void)fdef;
  return 32;     /* just the compulsory public members */
}

int  FieldSet(uint64 fdef, FIELD * f)
{
    if(fdef!=2) return -2;
    f[FDEF]=2;
    f[CHARC]=2;
    f[POW]=1;
    f[CONP]=1;
    return 1;
}

FELT FieldAdd(FIELD * f, FELT a, FELT b)
{
  (void)f;
    return a^b;
}

FELT FieldNeg(FIELD * f, FELT a)
{
  (void)f;
    return a;
}

FELT FieldSub(FIELD * f, FELT a, FELT b)
{
  (void)f;
    return a^b;
}

FELT FieldMul(FIELD * f, FELT a, FELT b)
{
  (void)f;
    return a&b;
}

FELT FieldInv(FIELD * f, FELT a)
{
  (void)f;
    return a;
}

FELT FieldDiv(FIELD * f, FELT a, FELT b)
{
  (void)f;
  (void)b;
    return a;
}

void DSSet(FIELD * f, uint64 noc, DSPACE * ds)
{
    ds->field=f;
    ds->noc=noc;
    ds->nob=((noc+63)/64)*8;
    ds->paktyp=1;
}

static uint64 getint64(FILE *f)
{
    unsigned char byt;
    uint64 u,x;
    int i,r;
    u=0;
    for(i=0;i<8;i++)
    {
	r=fread(&byt,1,1,f);
        if(r==99) printf("99\n");
        x=byt;
        x<<=8*i;
	u |= x;
    }
    return u;
}

static void putint64(FILE *f,  uint64 u)
{
    unsigned char byt;
    int i;
    for(i=0;i<8;i++)
    {
	byt = u & 255;
	fwrite(&byt,1,1,f);
	u = (u >> 8);
    }
}

FILE * RdHdr(char * fname, uint64 * fdef, uint64 * nor, uint64 * noc)
{
    FILE *f;
    f = fopen(fname,"rb");
    if(f == NULL)
    {
        printf("Cannot open input file %s\n",fname);
        exit(10);
    }
    *fdef = getint64(f);
    *nor  = getint64(f);
    *noc  = getint64(f);
    return f;
}

FILE * WrHdr(char * fname, uint64 fdef, uint64 nor, uint64 noc)
{
    FILE *f;
    f = fopen(fname,"wb");
    if(f == NULL)
    {
        printf("Cannot open output file %s\n",fname);
        exit(11);
    }
    putint64(f,fdef);
    putint64(f,nor);
    putint64(f,noc);
    return f;
}

void RdMatrix(FILE * f, DSPACE * ds, uint64 nor, Dfmt d)
{
    int r;
    r=fread(d,ds->nob,nor,f);
    if(r==99) printf("99\n");
}

void WrMatrix(FILE * f, DSPACE * ds, uint64 nor, Dfmt d)
{
    int r;
    r=fwrite(d,ds->nob,nor,f);
    if(r==99) printf("99\n");
}

void Close(FILE * f)
{
    fclose(f);
}


FELT DUnpak(DSPACE * ds, uint64 col, Dfmt d)
{
    uint8 * dp, x;
    FELT f;
    int i;
    (void)ds;
    dp=(uint8 *) d;
    dp+=(col>>3);
    x=*dp;
    i=col&7;
    x=(x>>i)&1;
    f=x;
    return f;
}

void DPak(DSPACE * ds, uint64 col, Dfmt d, FELT f)
{
    uint8 * dp, x,y,z;
    int i;
    (void)ds;
    dp=(uint8 *) d;
    dp+=(col>>3);
    x=*dp;
    i=col&7;
    y=f<<i;
    z=1<<i;
    x=x&(z^255);
    x|=y;
    *dp=x;
}

void DAdd(DSPACE * ds, Dfmt d1, Dfmt d2)
{
    uint64 *dp1, *dp2, imax, i;
    dp1=(uint64 *) d1;
    dp2=(uint64 *) d2;
    imax=ds->nob/8;
    for(i=0;i<imax;i++)  *(dp2++) ^= *(dp1++);
}

void DSMul(DSPACE * ds, FELT f, Dfmt d)
{
    if(f==0) memset(d,0,ds->nob);
}

void DMove(DSPACE * ds, Dfmt d1, Dfmt d2)
{
    memcpy(d2,d1,ds->nob);
}

void DCut(DSPACE * ds1, uint64 nor, uint64 col, Dfmt d1, DSPACE * ds2, Dfmt d2)
{
  (void)ds1;
  (void)nor;
  (void)col;
  (void)d1;
  (void)ds2;
  (void)d2;
}

void DPaste(DSPACE * ds1, Dfmt d1, uint64 nor, uint64 col,
                 DSPACE * ds2, Dfmt d2)
{
  uint64 fullw;           /* full words of input to process */
  uint64 bitslast;        /* number of bits to take from last word */
  uint64 *pt1, *pt2;      /* pointers to input and output */
  uint64 maske2;          /* to keep the bits after the pasted ones */
  uint64 maske1;          /* the bits from the input */
  uint64 i,j;             /* index for rows and fullwords */
  uint64 pt2add;          /* Fullwords to skip after paste */
  uint64 pt1add;          /* fullwords input to skip after fullw */

    fullw=ds1->noc>>6;
    bitslast=ds1->noc&63;
    maske2=0;
    if(bitslast!=0)
    {
        maske1=(1ull<<bitslast)-1;
        maske2=maske1^0xffffffffffffffffull;
    }
    pt1=(uint64 *) d1;
    pt2=(uint64 *) d2;
    pt2+=(col>>6);
    pt2add=((63+ds2->noc)>>6)-fullw;
    pt1add=((63+ds1->noc)>>6)-fullw;

    if( (col&63)==0 )       /* multiply case */
    {
        for(i=0;i<nor;i++)
        {
            for(j=0;j<fullw;j++)
                *(pt2++)=*(pt1++);
            if(bitslast!=0)     /* deal with the tatty-bit. */
                *pt2 = ((*pt2)&maske2) | (*pt1);
            pt2+=pt2add;
            pt1+=pt1add;
        }
    }
    else
    {
        printf("haven't written this bit yet\n");
        exit(113);
    }
}

/* end of field.c  */
