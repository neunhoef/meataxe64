/*
 * $Id: iv.c,v 1.9 2005/07/24 09:32:45 jon Exp $
 *
 * Invert a matrix
 *
 */

#include "iv.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "clean.h"
#include "elements.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "maps.h"
#include "matrix.h"
#include "memory.h"
#include "read.h"
#include "utils.h"
#include "write.h"

void invert(const char *m1, const char *m2, const char *name)
{
  FILE *inp, *outp;
  const header *h;
  u32 prime, nob, nor, len, n, r;
  word **mat1, **mat2;
  int *map1, *map2;
  int is_perm;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp, &h, m1, name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  is_perm = 1 == prime;
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  if (header_get_noc(h) != nor) {
    fprintf(stderr, "%s: matrix %s is not square, terminating\n", name, m1);
    fclose(inp);
    exit(1);
  }
  if (is_perm) {
    if (0 == map_iv(inp, h, m1, m2, name)) {
      fclose(inp);
      header_free(h);
      exit(1);
    }
    fclose(inp);
    header_free(h);
  } else {
    row_ops row_operations;
    grease_struct grease;
    rows_init(prime, &row_operations);
    len = header_get_len(h);
    r = memory_rows(len, 100);
    if (memory_rows(len, 400) < nor || r < prime) {
      fprintf(stderr, "%s: cannot allocate %u rows for %s and %s, terminating\n",
              name, 2 * (nor + prime), m1, m2);
      fclose(inp);
      header_free(h);
      exit(2);
    }
    (void)grease_level(prime, &grease, r);
    r = grease.level;
    /* Now read the matrix */
    mat1 = matrix_malloc(nor);
    mat2 = matrix_malloc(nor);
    for (n = 0; n < nor; n++) {
      mat1[n] = memory_pointer_offset(0, n, len);
      mat2[n] = memory_pointer_offset(400, n, len);
    }
    errno = 0;
    if (0 == endian_read_matrix(inp, mat1, len, nor)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, m1);
      header_free(h);
      fclose(inp);
      exit(1);
    }
    fclose(inp);
    /* Initialise mat2 */
    for (n = 0; n < nor; n++) {
      row_init(mat2[n], len);
      put_element_to_row(nob, n, mat2[n], 1);
    }
    echelise(&row_operations, mat1, nor, &n, &map1, mat2, 1, grease.level, prime, len, nob, 800, 900, len, 1, name);
    if (nor != n) {
      fprintf(stderr, "%s: matrix %s is singular with rank %u, terminating\n", name, m1, n);
      exit(1);
    }
    if (0 == open_and_write_binary_header(&outp, h, m2, name)) {
      exit(1);
    }
    header_free(h);
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
      errno = 0;
      if (0 == endian_write_row(outp, mat2[map2[r]], len)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot write row %u to %s, terminating\n", name, r, m2);
        fclose(outp);
        exit(1);
      }
    }
    matrix_free(mat1);
    matrix_free(mat2);
    free(map2);
    fclose(outp);
  }
}
