/*
 * $Id: rn.c,v 1.2 2001/11/12 13:43:38 jon Exp $
 *
 * Compute the rank of a matrix
 *
 */

#include "rn.h"
#include <stdio.h>
#include "clean.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "read.h"

unsigned int rank(const char *m, const char *name)
{
  FILE *inp;
  const header *h;
  unsigned int prime, nob, nor, len, n, r, grease_level, **mat, **new_mat;
  int *map;
  inp = fopen(m, "rb");
  if (NULL == inp) {
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, m);
    exit(1);
  }
  if (0 == read_binary_header(inp, &h, m)) {
    fclose(inp);
    exit(1);
  }
  prime = header_get_prime(h);
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  len = header_get_len(h);
  r = memory_rows(len, 100);
  if (memory_rows(len, 900) < nor || r < prime) {
    fprintf(stderr, "%s: cannot allocate %d rows for %s, terminating\n", name, nor + prime, m);
    fclose(inp);
    exit(1);
  }
  (void)grease(prime, &grease_level, r);
  /* Now read the matrix */
  if ( 0 == matrix_malloc(nor, (void **)&mat)) {
    fprintf(stderr, "%s: cannot allocate matrix for %s, terminating\n", name, m);
    fclose(inp);
    exit(1);
  }
  for (n = 0; n < nor; n++) {
    mat[n] = memory_pointer_offset(0, n, len);
  }
  if (0 == endian_read_matrix(inp, mat, len, nor)) {
    fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, m);
    fclose(inp);
    exit(1);
  }
  echelise(mat, nor, &n, &map, &new_mat, grease_level, prime, len, nob, 900);
  free(mat);
  free(map);
  free(new_mat);
  fclose(inp);
  return n;
}
