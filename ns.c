/*
 * $Id: ns.c,v 1.12 2004/08/21 13:22:30 jon Exp $
 *
 * Compute the null space of a matrix
 *
 */

#include "ns.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "clean.h"
#include "elements.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "parse.h"
#include "read.h"
#include "write.h"

unsigned int nullspace(const char *m1, const char *m2, const char *name)
{
  FILE *inp, *outp;
  const header *h1;
  header *h2;
  unsigned int prime, nob, nor, len1, len2, space1, space2, sub1, sub2, n, r, **mat1, **mat2;
  int *map;
  row_ops row_operations;
  grease_struct grease;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp, &h1, m1, name)) {
    exit(1);
  }
  prime = header_get_prime(h1);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
    fclose(inp);
    exit(1);
  }
  rows_init(prime, &row_operations);
  nob = header_get_nob(h1);
  nor = header_get_nor(h1);
  len1 = header_get_len(h1);
  h2 = header_create(prime, nob, header_get_nod(h1), nor, nor);
  len2 = header_get_len(h2);
  space1 = split_memory(len1, len2, 1000);
  space2 = 1000 - space1;
  if (verbose) {
    printf("%s: splitting memory %u : %u\n", name, space1, space2);
    fflush(stdout);
  }
  assert(10 <= space1 && space1 <= 990);
  assert(10 <= space2 && space2 <= 990);
  sub1 = space1 / 5;
  sub2 = space2 / 5;
  assert(0 != sub1 && 0 != sub2 && sub1 < space1 && sub2 < space2);
  r = memory_rows(len1, sub1);
  n = memory_rows(len2, sub2);
  header_free(h1);
  if (memory_rows(len1, space1 - sub1) < nor || memory_rows(len2, space2 - sub2) < nor || r < prime || n < prime) {
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
    mat1[n] = memory_pointer_offset(sub1 + sub2, n, len1);
    mat2[n] = memory_pointer_offset(sub2 /* + sub1 */ + space1 /* - sub1 */, n, len2);
  }
  errno = 0;
  if (0 == endian_read_matrix(inp, mat1, len1, nor)) {
    if ( 0 != errno) {
      perror(name);
    }
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
  echelise(&row_operations, mat1, nor, &n, &map, mat2, 1, grease.level, prime, len1, nob, 0, sub1, len2, 0, name);
  matrix_free(mat1);
  if (n < nor) {
    /* Output null rows of mat2 */
    header_set_nor(h2, nor - n);
    if (0 == open_and_write_binary_header(&outp, h2, m2, name)) {
      exit(1);
    }
    for (r = 0; r < nor; r++) {
      if (map[r] < 0) {
        errno = 0;
        if (0 == endian_write_row(outp, mat2[r], len2)) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s cannot write row %d to %s, terminating\n", name, r, m2);
          fclose(outp);
          exit(1);
        }
      }
    }
    fclose(outp);
  }
  header_free(h2);
  matrix_free(mat2);
  free(map);
  return nor - n;
}
