/*
 * $Id: iv.c,v 1.1 2001/12/15 20:47:27 jon Exp $
 *
 * Invert a matrix
 *
 */

#include "iv.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "clean.h"
#include "elements.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "read.h"
#include "utils.h"
#include "write.h"

void invert(const char *m1, const char *m2, const char *name)
{
  FILE *inp, *outp;
  const header *h;
  unsigned int prime, nob, nor, len, n, r, **mat1, **mat2;
  int *map1, *map2;
  grease_struct grease;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp, &h, m1, name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  if (header_get_noc(h) != nor) {
    fprintf(stderr, "%s: matrix %s is not square, terminating\n", name, m1);
    exit(1);
  }
  len = header_get_len(h);
  r = memory_rows(len, 100);
  if (memory_rows(len, 400) < nor || r < prime) {
    fprintf(stderr, "%s: cannot allocate %d rows for %s and %s, terminating\n",
            name, 2 * (nor + prime), m1, m2);
    fclose(inp);
    exit(2);
  }
  (void)grease_level(prime, &grease, r);
  r = grease.level;
  (void)grease_level(prime, &grease, n);
  if (r < grease.level) {
    /* Choose grease level compatible with both */
    grease.level = r;
  }
  /* Now read the matrix */
  mat1 = matrix_malloc(nor);
  mat2 = matrix_malloc(nor);
  for (n = 0; n < nor; n++) {
    mat1[n] = memory_pointer_offset(0, n, len);
    mat2[n] = memory_pointer_offset(400, n, len);
  }
  if (0 == endian_read_matrix(inp, mat1, len, nor)) {
    fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, m1);
    fclose(inp);
    exit(1);
  }
  fclose(inp);
  /* Initialise mat2 */
  for (n = 0; n < nor; n++) {
    row_init(mat2[n], len);
    put_element_to_row(nob, n, mat2[n], 1);
  }
  echelise(mat1, nor, &n, &map1, mat2, 1, grease.level, prime, len, nob, 800, 900, len, 1, name);
  if (nor != n) {
    fprintf(stderr, "%s matrix %s is singular with rank %d, terminating\n", name, m1, n);
    fclose(outp);
    exit(1);
  }
  if (0 == open_and_write_binary_header(&outp, h, m2, name)) {
    exit(1);
  }
  map2 = my_malloc(nor * sizeof(int));
  memset(map2, 0, nor * sizeof(int));
#ifndef NDEBUG
  for (r = 0; r < nor; r++) {
    map2[r] = -1;
  }
#endif
  for (r = 0; r < nor; r++) {
    assert(map1[r] < (int)nor && 0 <= map1[r]);
    map2[map1[r]] = r;
  }
#ifndef NDEBUG
  for (r = 0; r < nor; r++) {
    assert(0 <= map2[r]);
  }
#endif
  free(map1);
  for (r = 0; r < nor; r++) {
    if (0 == endian_write_row(outp, mat2[map2[r]], len)) {
      fprintf(stderr, "%s cannot write row %d to %s, terminating\n", name, r, m2);
      fclose(outp);
      exit(1);
    }
  }
  fclose(outp);
  header_free(h);
  matrix_free(mat1);
  matrix_free(mat2);
  free(map2);
}
