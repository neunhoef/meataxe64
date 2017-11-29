/*
            ztra.c    Thread farm based transpose 21.5.2016
            ======    J. G. Thackray amended by R. A. Parker
*/

#include <stdio.h>
#include <stdlib.h>
#include "field.h"
#include "tfarm.h"
#include "io.h"
#include "proggies.h"
#include "tuning.h"
 
int main(int argc, char **argv)
{
    int chz,cha;
    int fmoj,mja,mjb,mojmax,resmoj;
    int res,pri;
    int i,k;
    FIELD * f;
    uint64 HDR[5];
    uint64 fdef, nora, noca;
    LogCmd(argc,argv);
    if (argc != 3)
    {
        LogString(80,"usage ztr m1 ans");
        exit(21);
    }
// Take a peek at the input matrices to get dimensions and field
    EPeek(argv[1],HDR);
    fdef=HDR[1];
    nora=HDR[2];
    noca=HDR[3];
    cha=1;
    while((cha*cha)<THREADS*3) cha++;
    if(cha>MAXCHOP) cha=MAXCHOP;
    chz=cha;
    fmoj=0;
// moj 0 is FIELD
    mja=1; /* Matrix A */
// i chz j cha k chc
// moja[i][j] = mja+i*cha+j
    mjb=mja+chz*cha; /* mojb is offset from moja by the number of blocks */
// mojb[j][i] = mjb+j*chz+i
    mojmax=mjb+chz*cha; /* The largest moj (+1) */
    TfInitialize(mojmax,0,THREADS,2);
    TfGo();
    for(i=0;i<mojmax;i++) TfGetReadRef(i);

// Make the field moj
    f=TfAllocate(fmoj,FIELDLEN);
    res=FieldSet(fdef,f,0);
    (void)res;                   // Avoid compiler warnings
    TfMainMake(fmoj);

// Start the constitutive thread to read in A
    MxRead(argv[1],fmoj,mja,chz,cha,0); // Read A in unthrottled
// Start the constitutive thread to write out B unthrottled
    MxWrite(argv[2],fmoj,noca,nora,mjb,chz,cha,0); 
// Submit all the jobs
// i chz k cha
    for(i=0;i<chz;i++) {
      /* Loop over rows of A, columns of B */
      for(k=0;k<cha;k++) {
        /* Loop over columns of A, rows of B */
        resmoj=mjb + k * chz + i; /* B[j][i] = A[i][j] */
        pri=k;
        TfSubmit(pri,TRAPROG,fmoj,mja+i*cha+k,resmoj,-1);
      }
    }
    for(i=1;i<mojmax;i++) TfRelease(i);  // not fmoj
    TfWaitConst();
    TfRelease(fmoj);
    TfClose();
    return 0;
}
