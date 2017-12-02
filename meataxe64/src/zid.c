/*
      zid.c     meataxe-64 Generate an identity matrix
      =====     J. G. Thackray   30.10.2016 
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "field.h"
#include "io.h"
#include "slab.h"
 
int main(int argc,  char **argv)
{
  uint64_t fdef, nor, noc;
  uint64_t hdr[5];
  EFIL *e;
  FIELD *f;
  DSPACE ds;
  Dfmt *v1;
  uint64_t i;
  LogCmd(argc, argv);
  if (argc != 5) {
    LogString(80,"usage zid m1 fdef nor noc");
    exit(14);
  }
  fdef = strtoull(argv[2], NULL, 0);
  nor = strtoull(argv[3], NULL, 0);
  noc = strtoull(argv[4], NULL, 0);
  hdr[0] = 1;
  hdr[1] = fdef;
  hdr[2] = nor;
  hdr[3] = noc;
  hdr[4] = 0;
  e = EWHdr(argv[1],hdr);
  f = malloc(FIELDLEN);
  if (f == NULL) {
    LogString(81,"Can't malloc field structure");
    exit(8);
  }
  FieldSet(fdef, f);
  DSSet(f,noc,&ds);

  /******  check that a row fits in memory  */
  v1=malloc(ds.nob);
  if (v1 == NULL) {
    LogString(81,"Can't malloc a single vector");
    exit(9);
  }
  /******  for each row of the matrix  */
  for (i = 0; i < nor; i++) {
    memset(v1, 0, ds.nob);
    DPak(&ds, i, v1, 1);
    EWData(e, ds.nob, v1);
  }
  EWClose(e);
  free(v1);
  free(f);
  return 0;
}
