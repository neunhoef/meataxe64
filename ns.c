/*
 * $Id: ns.c,v 1.3 2001/11/21 01:06:29 jon Exp $
 *
 * Compute the null space of a matrix
 *
 */

#include "ns.h"
#include <stdio.h>
#include "clean.h"
#include "elements.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "read.h"
#include "write.h"

unsigned int nullspace(const char *m1, const char *m2, const char *name)
{
  FILE *inp, *outp;
  const header *h1;
  header *h2;
  unsigned int prime, nob, nor, len1, len2, n, r, **mat1, **mat2;
  int *map;
  grease_struct grease;
  inp = fopen(m1, "rb");
  if (NULL == inp) {
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, m1);
    exit(1);
  }
  if (0 == read_binary_header(inp, &h1, m1)) {
    fclose(inp);
    exit(1);
  }
  prime = header_get_prime(h1);
  nob = header_get_nob(h1);
  nor = header_get_nor(h1);
  len1 = header_get_len(h1);
  h2 = header_create(prime, nob, header_get_nod(h1), nor, nor);
  len2 = header_get_len(h2);
  r = memory_rows(len1, 100);
  n = memory_rows(len2, 100);
  if (memory_rows(len1, 400) < nor || memory_rows(len2, 400) < nor || r < prime || n < prime) {
    fprintf(stderr, "%s: cannot allocate %d rows for %s and %s, terminating\n",
            name, 2 * (nor + prime), m1, m2);
    fclose(inp);
    exit(1);
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
    mat1[n] = memory_pointer_offset(0, n, len1);
    mat2[n] = memory_pointer_offset(400, n, len2);
  }
  if (0 == endian_read_matrix(inp, mat1, len1, nor)) {
    fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, m1);
    fclose(inp);
    exit(1);
  }
  fclose(inp);
  /* Initialise mat2 */
  for (n = 0; n < nor; n++) {
    row_init(mat2[n], len2);
    put_element_to_row(nob, n, mat2[n], 1);
  }
  echelise(mat1, nor, &n, &map, mat2, 1, grease.level, prime, len1, nob, 800, 900, len2, 0, name);
  free(mat1);
  if (n < nor) {
    /* Output null rows of mat2 */
    outp = fopen(m2, "wb");
    if (NULL == outp) {
      fprintf(stderr, "%s: cannot open %s, terminating\n", name, m2);
      exit(1);
    }
    header_set_nor(h2, nor - n);
    if (0 == write_binary_header(outp, h2, m2)) {
      fclose(outp);
      exit(1);
    }
    for (r = 0; r < nor; r++) {
      if (map[r] < 0) {
        if (0 == endian_write_row(outp, mat2[r], len2)) {
          fprintf(stderr, "%s cannot write row %d to %s, terminating\n", name, r, m2);
          fclose(outp);
          return 0;
        }
      }
    }
    fclose(outp);
  }
  free(mat2);
  free(map);
  return nor - n;
}
