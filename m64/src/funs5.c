// Copyright (C) Richard Parker   2017
// Meataxe64 Nikolaus version funs5.c   Miscellanous small functions

// Contents
// fAdd
// fScalarMul
// fTrace
// fProjectiveVector
// fMulMatrixMap
// fMulMaps

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "field.h"
#include "io.h"
#include "funs.h"
#include "parse.h"
 
void fAdd(const char *fn1, int s1, const char *fn2, int s2,
          const char *fn3, int s3)
{
    EFIL *e1,*e2,*e3;
    FIELD * f;
    uint64_t hdr1[5],hdr2[5];
    uint64_t fdef,nor,noc;
    DSPACE ds;
    Dfmt *v1,*v2;
    uint64_t i;

    if (very_verbose) {
      printf("fAdd %s to %s giving %s\n", fn1, fn2, fn3);
    }
    e1=ERHdr(fn1,hdr1);
    e2=ERHdr(fn2,hdr2);

    if( (hdr1[1]!=hdr2[1]) || (hdr1[2]!=hdr2[2]) || (hdr1[3]!=hdr2[3]) )
    {
        LogString(80,"Matrices incompatible");
        exit(22);
    }
    fdef=hdr1[1];
    nor=hdr1[2];
    noc=hdr1[3];

    f = malloc(FIELDLEN);
    FieldASet1(fdef,f,NOMUL);
    e3 = EWHdr(fn3,hdr1);
    DSSet(f,noc,&ds);

    v1=malloc(ds.nob);
    v2=malloc(ds.nob);

/******  Do them one row at a time  */
    for(i=0;i<nor;i++)
    {
	ERData(e1,ds.nob,v1);
	ERData(e2,ds.nob,v2);
        DAdd(&ds,1,v1,v2,v2);
        EWData(e3,ds.nob,v2);
    }
    free(v1);
    free(v2);
    free(f);
    ERClose1(e1,s1);
    ERClose1(e2,s2);
    EWClose1(e3,s3);
    if (very_verbose) {
      printf("fAdd %s to %s giving %s done\n", fn1, fn2, fn3);
    }
    return;
}

void fScalarMul(const char *m1, int s1, const char *m2, int s2, FELT sc)
{
    EFIL *e1,*e2;
    uint64_t hdr[5];
    uint64_t fdef,nor,noc;
    DSPACE ds;
    Dfmt *v1;
    uint64_t i;
    FIELD *f;

    e1=ERHdr(m1,hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    f = malloc(FIELDLEN);
    FieldASet1(fdef,f,NOMUL);
    e2 = EWHdr(m2,hdr);
    DSSet(f,noc,&ds);

    v1=malloc(ds.nob);

/******  Do them one row at a time  */
    for(i=0;i<nor;i++)
    {
	ERData(e1,ds.nob,v1);
        DSMul(&ds,sc,1,v1);
        EWData(e2,ds.nob,v1);
    }
    free(v1);
    ERClose1(e1,s1);
    EWClose1(e2,s2);
    return;
}

FELT fTrace(const char *m1, int s1)
{
    EFIL *e;
    uint64_t fdef,nor,noc;
    uint64_t i,j,k;
    FIELD * f;
    Dfmt * v1;
    DSPACE ds;
    FELT fel,fel1;
    uint64_t hdr[5];

    e = ERHdr(m1,hdr);
    nor=hdr[2];
    noc=hdr[3];
    if(hdr[0]==3)
    {
        k=0;
        for(i=0;i<nor;i++)
        {
	    ERData(e,8,(uint8_t *) &j);
            if(i==j) k++;
        }
        ERClose1(e,s1);
        fel=(FELT) k;
        return k;
    }
    if(nor!=noc)
    {
        printf("Only square matrices have a trace\n");
        exit(30);
    }
    fdef=hdr[1];
    f = malloc(FIELDLEN);
    FieldASet1(fdef,f,NOMUL);
    DSSet(f,noc,&ds);
    v1=malloc(ds.nob);
    fel=0;
    for(i=0;i<nor;i++)
    {
	ERData(e,ds.nob,v1);
        fel1=DUnpak(&ds,i,v1);
        fel=FieldAdd(f,fel,fel1);
    }
    ERClose1(e,s1);
    free(f);
    free(v1);
    return fel;
}

void fProjectiveVector(const char *m1, int s1, uint64_t vecno,
                       const char *m2, int s2)
{
    EFIL *e1,*e2;
    uint64_t hdr[5];
    uint64_t fdef,nor,noc,i,old,depth,fpow;
    FIELD * f;
    DSPACE ds;
    Dfmt *m;
    Dfmt *v;
    Dfmt *vpt;
    FELT fel;
    e1 = ERHdr(m1,hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,noc,&ds);
    m=malloc(ds.nob*nor);
    v=malloc(ds.nob);
    ERData(e1,ds.nob*nor,m);
    ERClose1(e1,s1);
// work out what vector we want
    old=0;
    depth=0;
    fpow=1;
    while(vecno>=(old+fpow))
    {
        old+=fpow;
        fpow=fpow*fdef;
        depth++;
    }
    if(depth>=nor)
    {
        LogString(81,"Vector number outside range");
        exit(23);
    }
    vecno-=old;
// make that vector in v
    memset(v,0,ds.nob);
    for(i=0;i<depth;i++)
    {
        vpt=DPAdv(&ds,i,m);
        fel=vecno % fdef;
        DSMad(&ds,fel,1,vpt,v);
        vecno=vecno / fdef;
    }
    vpt=DPAdv(&ds,depth,m);
    DAdd(&ds,1,vpt,v,v);
    hdr[2]=1;
    e2 = EWHdr(m2,hdr);
    EWData(e2,ds.nob,v);
    EWClose1(e2,s2);
    free(f);
    free(m);
    free(v);
}

void fMulMatrixMap(const char *m1, int s1, const char *x2, int s2,
                       const char *m3, int s3)
{
    uint64_t fdef,nor,noc,norm,nocm,i,j,k;
    FIELD * f;
    EFIL *e1,*e2,*e3;
    uint64_t hdr1[5],hdr2[5];
    uint64_t * map;
    DSPACE ds1,ds3;
    Dfmt *v1,*v3;
    FELT fel1,fel2;
    int strat;
    
    e1 = ERHdr(m1,hdr1);
    fdef=hdr1[1];
    nor=hdr1[2];
    noc=hdr1[3];
    e2 = ERHdr(x2,hdr2);
    norm=hdr2[2];
    nocm=hdr2[3];
    if(norm!=noc)
    {
        printf("Matrix and map incompatible\n");
        exit(27);
    }
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,noc,&ds1);
    DSSet(f,nocm,&ds3);
    hdr1[3]=nocm;
    e3 = EWHdr(m3,hdr1);
// decide on strategy . . . currently only one!
// strat=1 one row at a time DUnpak DUnpak FieldAdd DPak
    strat=1;

    if(strat==1)
    {
        v1=malloc(ds1.nob);
        v3=malloc(ds3.nob);
        map=malloc(noc*sizeof(uint64_t));
        ERData(e2,noc*sizeof(uint64_t),(uint8_t *)map);
        ERClose1(e2,s2);
        for(i=0;i<nor;i++)
        {
            ERData(e1,ds1.nob,v1);
            memset(v3,0,ds3.nob);
            for(j=0;j<noc;j++)
            {
                k=map[j];
                fel1=DUnpak(&ds1,j,v1);
                fel2=DUnpak(&ds3,k,v3);
                fel1=FieldAdd(f,fel1,fel2);
                DPak(&ds3,k,v3,fel1);
            }
            EWData(e3,ds3.nob,v3);
        }
        ERClose1(e1,s1);
        EWClose1(e3,s3);   
    }
}

void fMulMaps(const char *x1, int s1, const char *x2, int s2,
                       const char *m3, int s3)
{
    EFIL *e1,*e2,*e3;
    uint64_t nor1,noc1,nor2,noc2,i;
    uint64_t hdr1[5],hdr2[5];
    uint64_t * map;
    uint64_t v1,v3;
    int strat;

    e1 = ERHdr(x1,hdr1);
    nor1=hdr1[2];
    noc1=hdr1[3];
    e2 = ERHdr(x2,hdr2);
    nor2=hdr2[2];
    noc2=hdr2[3];
    if(nor2!=noc1)
    {
        printf("Maps incompatible\n");
        exit(27);
    }
    hdr1[3]=noc2;
    e3 = EWHdr(m3,hdr1);
// decide on strategy . . . currently only one!
// strat=1 read in 2 then read 1 / write 3 one entry at a time
    strat=1;

    if(strat==1)
    {
// read in map 2
        map=malloc(nor2*sizeof(uint64_t));
        ERData(e2,nor2*sizeof(uint64_t),(uint8_t *)map);
        ERClose1(e2,s2);
// perform map composition
        for(i=0;i<nor1;i++)
        {
            ERData(e1,8,(uint8_t *)&v1);
            v3=map[v1];
            EWData(e3,8,(uint8_t *)&v3);
        }
// cleanup
        ERClose1(e1,s1);
        EWClose1(e3,s3);
        free(map);
    }
}

uint64_t pseed;
uint64_t pseed1;
uint64_t pseed2;
static uint64_t prand(void)
{
    uint64_t x;
    x=1;
    x=x<<63;
    if((x&pseed)!=0) pseed=(pseed<<1)^0x3b4f0bf89;
    else pseed=pseed<<1;
    /* 17 and 19 are not primitive roots. Changed to 2 and 5 */
    /* 2 and 5 don't work very well either (though better then 17 and 19) */
    /* Switched to 101 and 5, or maybe even 101 and 103 */
    /* They don't work, tried 1021 and 2063, which fail identically */
    /* Now try a different prime, 1000037, such that 1000002 and 1000036
     * have no common factor except 2 */
    pseed1=(pseed1*1021)%1000003;
    pseed2=(pseed2*2063)%1000037;
    return pseed+pseed1+pseed2;
}

void fRandomMatrix(const char *m1, int s1, uint64_t fdef, 
                   uint64_t nor, uint64_t noc, uint64_t seed, uint64_t count)
{
    EFIL *e1;
    uint64_t hdr[5];
    FIELD * f;
    DSPACE ds;
    Dfmt * v;
    uint64_t i,j;
    FELT elt;

    for(i = 0; i < count; i++) {
      prand();
    }
    hdr[0]=1;
    hdr[1]=fdef;
    hdr[2]=nor;
    hdr[3]=noc;
    hdr[4]=0;
    e1 = EWHdr(m1,hdr);
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,noc,&ds);
    v=malloc(ds.nob);
    pseed=seed;
    pseed1=pseed;
    pseed2=pseed;
    for(i=0;i<nor;i++)
    {
        memset(v,0,ds.nob);
	DPak(&ds,i,v,1);      // identity matrix bit
        for(j=nor;j<noc;j++)
        {
            elt=prand()%fdef;
	    DPak(&ds,j,v,elt);
        }
        EWData(e1,ds.nob,v);
    }
    EWClose1(e1,s1);
    free(f);
    free(v);
}

/******  end of funs5.c    ******/
