/*
 * $Id: rn.c,v 1.11 2002/01/06 16:35:48 jon Exp $
 *
 * Compute the rank of a matrix
 *
 */

#include "rn.h"
#include <stdio.h>
#include <assert.h>
#include "clean.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "read.h"

int rank(const char *m, unsigned int *r, const char *name)
{
  FILE *inp;
  const header *h;
  unsigned int prime, nob, nor, len, n, **mat;
  int *map;
  grease_struct grease;
  assert(NULL != m);
  assert(NULL != name);
  assert(NULL != r);
  if (0 == open_and_read_binary_header(&inp, &h, m, name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  len = header_get_len(h);
  header_free(h);
  n = memory_rows(len, 100);
  if (memory_rows(len, 900) < nor || n < prime) {
    fprintf(stderr, "%s: cannot allocate %d rows for %s, terminating\n", name, nor + prime, m);
    fclose(inp);
    exit(2);
  }
  (void)grease_level(prime, &grease, n);
  /* Now read the matrix */
  mat = matrix_malloc(nor);
  for (n = 0; n < nor; n++) {
    mat[n] = memory_pointer_offset(0, n, len);
  }
  if (0 == endian_read_matrix(inp, mat, len, nor)) {
    fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, m);
    fclose(inp);
    return 0;
  }
  echelise(mat, nor, r, &map, NULL, 0, grease.level, prime, len, nob, 900, 0, 0, 0, name);
  matrix_free(mat);
  free(map);
  fclose(inp);
  return 1;
}

