/*
 * Compute the determinant of a matrix
 *
 */

#include "det.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "clean.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "maps.h"
#include "matrix.h"
#include "memory.h"
#include "parse.h"
#include "primes.h"
#include "psign.h"
#include "read.h"
#include "utils.h"

static word sign_to_elt(int sign, word ch)
{
  return (1 == sign) ? 1 : ch - 1; /* map 1 -> 1, -1 -> p-1 */
}

int det(const char *m, word *d, const char *name)
{
  FILE *inp;
  const header *h;
  u32 prime, nob, nor, len, n, ch;
  word **mat;
  int is_perm;
  assert(NULL != m);
  assert(NULL != name);
  assert(NULL != d);
  if (0 == open_and_read_binary_header(&inp, &h, m, name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  is_perm = (1 == prime);
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  len = header_get_len(h);
  ch = prime_divisor(prime);
  header_free(h);
  if (is_perm) {
    word *map = malloc_map(nor);
    int sign;
    if (0 == read_map(inp, nor, map, name, m)) {
      map_free(map);
      return 0;
    }
    sign = psign_value(map, nor);
    *d = sign_to_elt(sign, ch);
  } else {
    row_ops row_operations;
    prime_ops prime_operations;
    grease_struct grease;
    word sign;
    u32 r;
    int *int_map;
    word *map = my_malloc(nor * sizeof(*map));
    primes_init(prime, &prime_operations);
    rows_init(prime, &row_operations);
    n = memory_rows(len, 100);
    if (memory_rows(len, 900) < nor || n < prime) {
      fprintf(stderr, "%s: cannot allocate %u rows for %s, terminating\n", name, nor + prime, m);
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
    *d = 1;
    echelise_with_det(&row_operations, mat, nor, &r, &int_map, d, NULL, 0, grease.level, prime, len, nob, 900, 0, 0, 0, name);
    if (nor != r) {
      /* Singular */
      *d = 0;
    } else {
      u32 i;
      for (i = 0; i < nor; i++) {
        map[i] = int_map[i];
      }
      sign = sign_to_elt(psign_value(map, nor), ch);
      *d = prime_operations.mul(*d, sign);
    }
    matrix_free(mat);
    free(map);
  }
  fclose(inp);
  return 1;
}

int det2(const char *m, word *d, const char *name)
{
  FILE *inp;
  const header *h;
  u32 prime, nob, nor, len, n;
  word **mat;
  int is_perm;
  assert(NULL != m);
  assert(NULL != name);
  assert(NULL != d);
  if (0 == open_and_read_binary_header(&inp, &h, m, name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  is_perm = (1 == prime);
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  len = header_get_len(h);
  header_free(h);
  if (is_perm) {
    return 0; /* Det2 isn't for permutations */
  } else {
    row_ops row_operations;
    prime_ops prime_operations;
    grease_struct grease;
    word sign;
    u32 r;
    int *int_map;
    word *map = my_malloc(nor * sizeof(*map));
    primes_init(prime, &prime_operations);
    rows_init(prime, &row_operations);
    n = memory_rows(len, 100);
    if (memory_rows(len, 900) < nor || n < prime) {
      fprintf(stderr, "%s: cannot allocate %u rows for %s, terminating\n", name, nor + prime, m);
      fclose(inp);
      exit(2);
    }
    max_grease = 1;
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
    *d = 1;
    echelise_with_det2(&row_operations, mat, nor, &r, &int_map, d, NULL, 0, grease.level, prime, len, nob, 900, 0, 0, 0, name);
    if (nor != r) {
      /* Singular */
      *d = 0;
    } else {
      u32 i;
      for (i = 0; i < nor; i++) {
        map[i] = int_map[i];
      }
      sign = sign_to_elt(psign_value(map, nor), prime);
      *d = prime_operations.mul(*d, sign);
    }
    matrix_free(mat);
    free(map);
  }
  fclose(inp);
  return 1;
}
