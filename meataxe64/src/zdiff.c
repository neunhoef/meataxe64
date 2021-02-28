/*
      zdiff.c     meataxe-64 compare matrices
      =======     J. G. Thackray   27.02.2020
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "field.h"
#include "io.h"
#include "mfuns.h"

static const char prog_name[] = "zdiff";

static void script_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in file 1> <in file 2>\n", prog_name, prog_name);
}

extern EFIL * ERHdr(const char * fname, uint64_t * header);

int main(int argc, const char *argv[])
{
  int ok = 0; /* Assume we'll succeed */
  EFIL *f1, *f2;
  header h1, h2;
  DSPACE m1;
  Dfmt *v1, *v2;
  FIELD *f;
  uint64_t i;
  CLogCmd(argc, argv);
  if (argc != 3) {
    script_usage();
    exit(1);
  }
  /* Open files */
  f1 = ERHdr(argv[1], h1.hdr);
  if (NULL == f1) {
    fprintf(stderr, "%s: cannot open %s\n", prog_name, argv[1]);
    exit(1);
  }
  f2 = ERHdr(argv[2], h2.hdr);
  if (NULL == f2) {
    fprintf(stderr, "%s: cannot open %s\n", prog_name, argv[2]);
    exit(1);
  }
  /* Check for perms */
  if (1 == h1.named.fdef) {
    if (1 != h2.named.fdef) {
      fprintf(stderr, "%s: cannot compare permutation %s with non permutation %s\n", prog_name, argv[1], argv[2]);
      exit(1);
    }
    /* Do perms case */
    /* Perms case: check dimension */
    /* Perms case: check elements */
  } else {
    if (1 == h2.named.fdef) {
      fprintf(stderr, "%s: cannot compare non permutation %s with permutation %s\n", prog_name, argv[1], argv[2]);
      exit(1);
    }
    /* Non perms case: check fdef and dimension */
    if (h1.named.fdef != h2.named.fdef ||
        h1.named.nor != h2.named.nor ||
        h1.named.noc != h2.named.noc) {
      fprintf(stderr, "%s: cannot compare incompatible matrices %s and %s\n", prog_name, argv[1], argv[2]);
      exit(1);
    }
    f = malloc(FIELDLEN);
    FieldASet(h1.named.fdef, f);
    DSSet(f, h1.named.noc, &m1);
    v1 = malloc(m1.nob);
    v2 = malloc(m1.nob);
    /* Check row by row */
    for (i = 0; i < h1.named.nor; i++) {
      ERData(f1, m1.nob, v1);
      ERData(f2, m1.nob, v2);
      if (0 != memcmp(v1, v2, m1.nob)) {
        ok = 1;
        break;
      }
    }
  }
  if (0 == ok) {
    ERClose(f1);
    ERClose(f2);
  }
  return ok;
}
