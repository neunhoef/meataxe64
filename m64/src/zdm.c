/*
      zdm.c     meataxe-64 Matrix dot multiply program
      =====     J. G. Thackray 19.09.21
      This is a technical program to assists with computation of orthogonal
      group signs. Its purpose is to compute the X^n-2 coefficient of
      the characteristic polynomial of its argument.
      This is the sum of the determinants over i<j of
      Xii Xij
      Xji Xjj
      In order to ease this, we first supply a transposed version of X
      as a second argument, meaning we can get the XijXji part by
      going along rows of both files and multiplying the corresponding
      elements, a bit like a dot product (hence the name).
      We accumulate the diagonals as we got and do the XiiXjj sum at the end
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "io.h"
#include "field.h"
#include "funs.h"
#include "mfuns.h"

static FELT fDM(const char * fn1, int s1, const char * fn2, int s2)
{
  EFIL *e1, *e2;
  FIELD *f;
  header h1, h2;
  uint64_t fdef,nor,noc;
  DSPACE ds;
  Dfmt *v1, *v2, *v3, *v4;
  uint64_t i, j;
  FELT e = 0;

  e1 = ERHdr(fn1, h1.hdr);
  e2 = ERHdr(fn2, h2.hdr);

  if ((h1.named.fdef != h2.named.fdef) || (h1.named.nor != h2.named.nor) || (h1.named.noc != h2.named.noc)) {
    LogString(80,"Matrices incompatible");
    exit(22);
  }
  fdef = h1.named.fdef;
  nor = h1.named.nor;
  noc = h1.named.noc;

  f = malloc(FIELDLEN);
  FieldASet1(fdef, f, NOMUL);
  DSSet(f, noc, &ds);

  v1 = malloc(ds.nob);
  v2 = malloc(ds.nob);
  v3 = malloc(ds.nob);
  v4 = malloc(ds.nob);

  /******  Do them one row at a time  */
  for (i = 0; i < nor; i++) {
    /*
     * Note this loop goes up to nor
     * We need to read the final row to get the final diagonal elements
     */
    ERData(e1, ds.nob, v1);
    ERData(e2, ds.nob, v2);
    for (j = i + 1; j < nor; j++) {
      FELT e1, e2;
      e1 = DUnpak(&ds, j, v1);
      e2 = DUnpak(&ds, j, v2);
      e = FieldAdd(f, e, FieldMul(f, e1, e2));
      /*printf("Off diagonal i %lu, j %lu, e1 %lu, e2 %lu, e %lu\n", i, j, e1, e2, e);*/
    }
    /* Now populate the diagonal elements */
    DPak(&ds, i, v3, DUnpak(&ds, i, v1));
    DPak(&ds, i, v4, DUnpak(&ds, i, v2));
  }
  /* Now negate e, det = ad - bc and we've done the bc bit */
  e = FieldNeg(f, e);
  /* Now add in the sum over the diagonal elements */
  for (i = 0; i < nor - 1; i++) {
    for (j = i + 1; j < nor; j++) {
      FELT e1, e2;
      e1 = DUnpak(&ds, i, v3);
      e2 = DUnpak(&ds, j, v4);
      e = FieldAdd(f, e, FieldMul(f, e1, e2));
      /*printf("On diagonal i %lu, j %lu, e1 %lu, e2 %lu, e %lu\n", i, j, e1, e2, e);*/
    }
  }
  free(v1);
  free(v2);
  free(v3);
  free(v4);
  free(f);
  ERClose1(e1, s1);
  ERClose1(e2, s2);
  return e;
}

int main(int argc,  char **argv)
{
  LogCmd(argc,argv);
  if (argc != 3) {
    LogString(80,"usage zdm <m1> <m2>");
    exit(14);
  }
  printf(" %lu\n", fDM(argv[1], 0, argv[2], 0));
  return 0;
}

/* end of zad.c  */
