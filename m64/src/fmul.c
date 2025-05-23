// Copyright (C) Richard Parker 2019
// Meataxe64 Early Diskchop version
// fmul.c  fMultiply function  

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "mezz.h"
#include "field.h"
#include "io.h"
#include "funs.h"
#include "slab.h"
#include "mfuns.h"
#include "utils.h"
#include "parse.h"

void fMultiply(const char *tmp, const char *m1, int s1, 
               const char *m2, int s2, const char *m3, int s3)
{
    uint64_t hdr1[5],hdr2[5];
    FIELD * f;
    EFIL *e1,*e2,*e3;
    Dfmt *am,*bm,*cm;
    uint64_t fdef,siz,sizac,sizb,chops,nor1,noc1,nor2,noc2;
    DSPACE ds1,ds2;
    uint64_t i, j;
    EPeek(m1,hdr1);
    EPeek(m2,hdr2);
    if (very_verbose) {
      printf("fMultiply %s by %s giving %s\n", m1, m2, m3);
    }
    if( (hdr1[0]==1) && (hdr2[0]==1) )  //flat matrix multiply
    {
        fdef=hdr1[1];
        f = malloc(FIELDLEN);
        FieldSet(fdef,f);
        nor1=hdr1[2];
        noc1=hdr1[3];
        nor2=hdr2[2];
        noc2=hdr2[3];
        if( (noc1!=nor2) || (fdef!=hdr2[1]) )
        {
            printf("Matrices incompatible\n");
            exit(27);
        }
        DSSet(f,noc1,&ds1);
        DSSet(f,noc2,&ds2);
// first look if matrix "small" - N^3 < 10^6
        siz=nor1*noc1;
        if(siz<=1000000) siz=siz*noc2;   // avoid overflow
        if(siz<1000000)   // small matrix - do in using slab;
        {
            e1=ERHdr(m1,hdr1);
            e2=ERHdr(m2,hdr2);
            hdr2[2]=nor1;
            e3=EWHdr(m3,hdr2);
            am=malloc(ds1.nob*nor1);
            bm=malloc(ds2.nob*nor2);
            cm=malloc(ds2.nob*nor1);
            ERData(e1,ds1.nob*nor1,am);
            ERClose1(e1,s1);
            ERData(e2,ds2.nob*nor2,bm);
            ERClose1(e2,s2);
            SLMul(f,am,bm,cm,nor1,noc1,noc2);
            EWData(e3,ds2.nob*nor1,cm);
            EWClose1(e3,s3);
            free(f);
            free(am);
            free(bm);
            free(cm);
            if (very_verbose) {
              printf("fMultiply %s by %s giving %s done\n", m1, m2, m3);
            }
            return;
        }
//  Consider disk chopping
//  if B, or A+C fit in memory,mmul is the way to go
        sizb=ds2.nob*nor2;
        sizac=(ds1.nob+ds2.nob)*nor1;
        siz=sizb;
        if(sizac<siz) siz=sizac;    // siz is smaller of B, A+C
        siz=siz/f->megabytes;
        siz=siz/250000;        /* We need 4 times as much memory as the files */
        chops=1;
        while((chops*chops)<=siz) chops++;
        if(chops==1)   // chopping into one piece!
        {
            mmul(m1,s1,m2,s2,m3,s3);
            if (very_verbose) {
              printf("fMultiply %s by %s giving %s done\n", m1, m2, m3);
            }
            free(f);
            return;
        }
        /* Extensions of temporary names for split multiply */
#define IN_EXT "_fmuli"
#define OUT_EXT "_fmulo"
        i = strlen(tmp);
        {
          uint64_t k;
          char *tmpi = malloc(i + strlen(IN_EXT) + 2 + 1);
          char *tmpo = malloc(i + strlen(OUT_EXT) + 2 + 1);
          /* Leave 2 spaces for digits */
          uint64_t count = chops * chops;
          if (count > 100) {
            count = 100; /* Just let it recurse */
          }
          strcpy(tmpi, tmp);
          strcat(tmpi, IN_EXT);
          strcpy(tmpo, tmp);
          strcat(tmpo, OUT_EXT);
          /*
           * Note we don't need to remember all the output names,
           * as splice only needs the root.
           * But we do need to create them to pass to fmul
           */
          slice(m1, count, tmpi);
          j = strlen(tmpi);
          k = strlen(tmpo);
          tmpi[j + 2] = 0;
          tmpo[k + 2] = 0;
          for (i = 0; i < count; i++) {
            /* Put the (same) extension into tmpi and tmpo */
            tmpi[j] = '0' + i / 10;
            tmpi[j + 1] = '0' + i % 10;
            tmpo[k] = '0' + i / 10;
            tmpo[k + 1] = '0' + i % 10;
            /* Use tmpi as tmp for next call */
            fMultiply(tmpi, tmpi, 1, m2, 1, tmpo, 1); /* Don't log */
          }
          tmpo[k] = 0; /* Terminate without the numbers */
          splice(tmpo, count, m3);
          /* Now delete all the temporaries */
          for (i = 0; i < count; i++) {
            /* Put the (same) extension into tmpi and tmpo */
            tmpi[j] = '0' + i / 10;
            tmpi[j + 1] = '0' + i % 10;
            tmpo[k] = '0' + i / 10;
            tmpo[k + 1] = '0' + i % 10;
            remove(tmpi);
            remove(tmpo);
          }
          free(tmpi);
          free(tmpo);
        }
        if (very_verbose) {
          printf("fMultiply %s by %s giving %s done\n", m1, m2, m3);
        }
        free(f);
        return;
    }
    if( (hdr1[0]==1) && (hdr2[0]==3) )
    {
        fMulMatrixMap(m1,s1,m2,s2,m3,s3);
        if (very_verbose) {
          printf("fMultiply %s by %s giving %s done\n", m1, m2, m3);
        }
        return;
    }
    if( (hdr1[0]==3) && (hdr2[0]==3) )
    {
        fMulMaps(m1,s1,m2,s2,m3,s3);
        if (very_verbose) {
          printf("fMultiply %s by %s giving %s done\n", m1, m2, m3);
        }
        return;
    }
    if((hdr1[0] == 3) && (hdr2[0]==1))    // map * matrix
    {
      /* This case is a PITA.
       * the code below attempts to read the entire right hand argument B
       * into real memory and then write it out permuted
       * This is obviously going to run out of memory when B is very large
       * Probably we should just write out uncompressed vectors into
       * a file, placing each at the correct offset,
       * and then read them back and write them using IO
       * To do this, we need to invert the permutation,
       * which we can do in memory.
       */
      uint64_t *inv = perm_inv(m1, &nor1);
      FILE *tmpo = fopen(tmp, "w");
      size_t junk;
      e2 = ERHdr(m2, hdr2);
      fdef = hdr2[1];
      f = malloc(FIELDLEN);
      FieldSet(fdef, f);
      nor2 = hdr2[2];
      noc2 = hdr2[3];
      DSSet(f, noc2, &ds2);
      /* A permutation has noc and nor equal, so noc can be disregarded */
      if(nor1 != nor2) {
        printf("map and matrix incompatible\n");
        exit(7);
      }
      hdr2[2] = nor1;
      e3 = EWHdr(m3, hdr2);
      /* Open a temporary */
      /* Allocate enough memory for a row from e2 */
      bm = malloc(ds2.nob);
      /* Loop reading a row from e2 and writing to the correct place in emp */
      for(i = 0; i < nor1; i++) {
        j = inv[i];
        /* Read a row */
        ERData(e2, ds2.nob, bm);
        /* Place in correct place in tmp */
        fseek(tmpo, j * ds2.nob, SEEK_SET);
        junk = fwrite(bm, 1, ds2.nob, tmpo);
      }
      ERClose1(e2, s2);
      fclose(tmpo);
      tmpo = fopen(tmp, "r");
      /* Copy the temp back to e3, using IO to compress etc */
      for(i = 0; i < nor1; i++) {
        /* Read a row from tmp into bm */
        junk = fread(bm, 1, ds2.nob, tmpo);
        /* Write into output */
        EWData(e3, ds2.nob, bm);
      }
      NOT_USED(junk);
      fclose(tmpo);
      remove(tmp);
      EWClose1(e3, s3);
      free(f);
      free(bm);
      if (very_verbose) {
        printf("fMultiply %s by %s giving %s done\n", m1, m2, m3);
      }
      return;
    }
    printf("fMultiply cannot handle these matrix types\n");
    exit(20);
}

/* end of fmul.c  */
