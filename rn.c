/*
 * $Id: rn.c,v 1.14 2004/04/25 16:31:48 jon Exp $
 *
 * Compute the rank of a matrix
 *
 */

#include "rn.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "clean.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "maps.h"
#include "matrix.h"
#include "memory.h"
#include "read.h"

int rank(const char *m, unsigned int *r, const char *name)
{
  FILE *inp;
  const header *h;
  unsigned int prime, nob, nor, len, n, **mat;
  int *map;
  int is_perm;
  assert(NULL != m);
  assert(NULL != name);
  assert(NULL != r);
  if (0 == open_and_read_binary_header(&inp, &h, m, name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  is_perm = (1 == prime);
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  len = header_get_len(h);
  if (is_perm) {
    if (0 == map_rank(inp, h, m, r, name)) {
      fclose(inp);
      header_free(h);
      exit(1);
    }
    header_free(h);
  } else {
    row_ops row_operations;
    grease_struct grease;
    rows_init(prime, &row_operations);
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
    errno = 0;
    if (0 == endian_read_matrix(inp, mat, len, nor)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, m);
      fclose(inp);
      return 0;
    }
    echelise(&row_operations, mat, nor, r, &map, NULL, 0, grease.level, prime, len, nob, 900, 0, 0, 0, name);
    matrix_free(mat);
    free(map);
  }
  fclose(inp);
  return 1;
}
