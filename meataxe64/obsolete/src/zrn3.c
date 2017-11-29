/*
    zrn3.c meataxe-64 proggy based rank program Rank program
    ====== J G Thackray 13.08.15  R A Parker 15.5.16  ======
*/

#include <stdio.h>
#include <stdlib.h>
#include "field.h"
#include "io.h"
#include "tfarm.h"
#include "proggies.h"
#include "tuning.h"

#ifndef CHOP_SIZE
#define CHOP_SIZE MAXCHOP
#endif

#define PRI1 ((i)+(j))
#define PRI2 ((i)+(k))

static int *al2d(unsigned int chop_size)
{
    int *res;
    res = malloc(sizeof(int *) * MAXCHOP * chop_size );
    if (res == NULL)
    {
        fprintf(stderr, "Failed to allocate 2d moj array, exiting\n");
        exit(1);
    }
    return res;
}

#define A2D(a,b,c) ((a)+(b)*MAXCHOP+(c))

int main(int argc, char **argv)
{
    FIELD *f;
    uint64 hdr[5];
    uint64 fdef, nor, noc;
    int i, j, k, m;
    char st[200];
    int fmoj = 0;
    int res, rank = 0;
/* 2 d mojes */
    int A,D,E,G,H,K,L,M,N,Q,U,S,T,V,W,X,Z;
    int *P_moj;
    int *R_moj;
    int *B_moj;
    int *C_moj;
    int *C_orig;
    LogCmd(argc,argv);
    if (argc != 2)
    {
        LogString(80,"usage zrn <m1>");
        exit(14);
    }
/*
* Need to allow for some overflow mojes
* We use up to 4 per pass through, so add 4 * CHOP_SIZE ** 3
*/
    TfInitialize( 1 + 6 * CHOP_SIZE * CHOP_SIZE * CHOP_SIZE + 4 * CHOP_SIZE * CHOP_SIZE,
           (18 * CHOP_SIZE + 12) * CHOP_SIZE * CHOP_SIZE, THREADS, 0); /*  no PVs */
/* Allocate the 2d moj arrays */
    P_moj = malloc(CHOP_SIZE*sizeof(int));
    R_moj = malloc(CHOP_SIZE*sizeof(int));
/* Allocate the 3d moj arrays */
    B_moj = al2d(CHOP_SIZE);
    C_moj = al2d(CHOP_SIZE);
    C_orig= al2d(CHOP_SIZE);
/* Now peek the input to get the noc, nor, field */
    EPeek(argv[1], hdr);
    fdef = hdr[1];
    nor = hdr[2];
    noc = hdr[3];
    if ( (nor==0) || (noc==0) )
    {
        sprintf(st,"Rank of %s is 0", argv[1]);
        LogString(20, st);
        return 0;
    }
    f = TfAllocate(fmoj, FIELDLEN);
    res = FieldSet(fdef, f, 0);
    (void)res;
    TfSetOutput(fmoj);
    TfStable(fmoj);
/* Start thread to read mat */
    MxRead1(argv[1], fmoj, C_moj, CHOP_SIZE, CHOP_SIZE, 0); /* Read mat in unthrottled */
    for(i=0;i<CHOP_SIZE;i++)
        for(j=0;j<CHOP_SIZE;j++)
            *A2D(C_orig,i,j)=*A2D(C_moj,i,j);
    for (i = 0; i < CHOP_SIZE; i++) /* Row echelise count in i */
    {
        for (j = 0; j < CHOP_SIZE; j++) /* Column echelise count in j */
        {
/* Special handling for i = 0 */
            if (0 == i)
            {
                H = *A2D(C_moj,i,j);
                ECH(PRI1, fmoj, H, T, 
                     P_moj[j], M, K,
                     R_moj[j]);
/* Ensure we don't throw away the answer before use */
                if(i==(CHOP_SIZE-1)) TfGetReadRef(P_moj[j]);
            } 
            else 
            {
                CEX(PRI1, fmoj, P_moj[j], *A2D(C_moj,i,j), A, G);
                MAD(PRI1, fmoj, A, R_moj[j], G,H);
                ECH(PRI1, fmoj, H, T, Q, M, K, N);
                CEX(PRI1, fmoj, Q, R_moj[j], E, D);
                MAD(PRI1, fmoj, E, N, D, L);
                PVC(PRI1, P_moj[j], Q, P_moj[j], U);
                RRF(PRI1, fmoj, U, L, N, R_moj[j]);
/* Ensure we don't throw away the answer before use */
                if(i==(CHOP_SIZE-1)) TfGetReadRef(P_moj[j]);
            }
/*
* Now we propagate these results right and down from above
* These are the cubic parts.
* They don't happen on the final loo,
* as there's nothing to do.
*/
            for (k = j + 1; k < CHOP_SIZE; k++)
            {
                m = j + 1;
/* Special handling for first row */
                if (0 == i)
                {
/* Z1jk = C1jk = Y1j, hence use of c_moj below */
                    REX(PRI2, fmoj, T, *A2D(C_moj,i,k), V, W);
                    MUL(PRI2, fmoj, M, V, X);
                    MCP(PRI2, fmoj, X, *A2D(B_moj,j,k));
                } 
                else
                {
                    MAD(PRI2, fmoj, A,*A2D(B_moj,j,k), *A2D(C_moj,i,k), Z);
                    REX(PRI2, fmoj, T, Z, V, W);
                    MUL(PRI2, fmoj, M, V, X);
                    MAD(PRI2, fmoj, E, X,*A2D(B_moj,j,k), S);
                    RRF(PRI2, fmoj, U, S, X,*A2D(B_moj,j,k));
                }
                if(m<CHOP_SIZE)
                    MAD(PRI2, fmoj, K, V, W, *A2D(C_moj,i,k));
            }
        }
    }
    for(i=0;i<CHOP_SIZE;i++)
        for(j=0;j<CHOP_SIZE;j++)
            TfRelease(*A2D(C_orig,i,j));
/* Allow things to start */
    TfGo();

/* Now wait for the results and compute the answer */
    i=CHOP_SIZE-1;
    for (j = 0; j < CHOP_SIZE; j++)
    {
        uint64 *cs;
        TfWait(P_moj[j]);
        cs = TfPointer(P_moj[j]);
        rank += cs[1];/* Add in number of set bits */
        TfRelease(P_moj[j]);
    }
    sprintf(st,"Rank of %s is %d", argv[1], rank);
    printf("%s\n", st);
    LogString(20, st);
    TfRelease(fmoj);
    TfClose();
    free(P_moj);
    free(R_moj);
    free(B_moj);
    free(C_moj);
    free(C_orig);
    return 0;
}

/* end of zrn3.c  */
