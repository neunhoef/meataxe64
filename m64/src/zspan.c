/*
 * Compute the span of a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "endian.h"
#include "span.h"
#include "parse.h"
#include "utils.h"
#include "field.h"
#include "io.h"
#include "mfuns.h"
#include "slab.h"

static const char *name = "zspan";

static void span_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file> <n>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  EFIL *inp;
  EFIL *outp;
  FIELD *f;
  DSPACE ds;
  Dfmt *am, *row;
  u64 prime, noc, nor, rows;
  u32 i, vectors;
  header h_in, h_out;
  Dfmt **mat;
  word *scalars;

  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    span_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  vectors = strtoul(argv[3], NULL, 0);
  if (0 == vectors) {
    vectors = 1;
  }
  inp = ERHdr(in, h_in.hdr);
  prime = h_in.named.fdef;
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
    ERClose(inp);
    exit(1);
  }
  noc = h_in.named.noc;
  nor = h_in.named.nor;
  f = malloc(FIELDLEN);
  FieldSet(prime, f);
  DSSet(f, noc, &ds);
  if (0 == int_pow(prime, vectors, &rows)) {
    fprintf(stderr, "%s: cannot compute %lu ** %u, terminating\n", name, prime, vectors);
    exit(1);
  }
  rows--;
  rows /= (prime - 1); /* Only want projective representatives */
  h_out.named.fdef = prime;
  h_out.named.nor = rows;
  h_out.named.noc = noc;
  outp = EWHdr(out, h_out.hdr);
  mat = malloc(vectors * sizeof(word));
  am = malloc(ds.nob * nor);
  for (i = 0; i < vectors; i++) {
    mat[i] = am + i * ds.nob;
  }
  ERData(inp, ds.nob * nor, am);
  row = malloc(ds.nob);
  scalars = my_malloc(vectors * sizeof(word));
  memset(scalars, 0, vectors * sizeof(word));
  for (i = 0; i < rows; i++) {
    u32 j;
    span(vectors, scalars, prime, &j);
    memset(row, 0, ds.nob);
    for (j = 0; j < vectors; j++) {
      if (0 != scalars[j]) {
        if (1 != scalars[j]) {
          /* Scaled add; does m64 have this? */
          DSMad(&ds, scalars[j], 1, mat[j], row);
        } else {
          /* Row add */
          DAdd(&ds, 1, mat[j], row, row);
        }
      }
    }
    EWData(outp, ds.nob, row);
  }
  ERClose(inp);
  EWClose(outp);
  free(mat);
  return 0;
}
