/*
 * $Id: mul.c,v 1.23 2002/04/10 23:33:27 jon Exp $
 *
 * Function to multiply two matrices to give a third
 *
 */

#include "mul.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "elements.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "maps.h"
#include "matrix.h"
#include "memory.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

#define M1_SIZE 450
#define M2_SIZE 100

#define contract(elts,prime,nob) ((2 == (prime)) ? (elts) : elements_contract(elts, prime, nob))

static int cleanup(FILE *inp1, FILE *inp2, FILE *outp)
{
  if (NULL != inp1)
    fclose(inp1);
  if (NULL != inp2)
    fclose(inp2);
  if (NULL != outp)
    fclose(outp);
  return 0;
}

static int mul_from_maps(FILE *inp1, FILE *inp2, const header *h1, const header *h2,
                         const char *m1, const char *m2, const char *m3, const char *name)
{
  unsigned int nor1, nor2, noc2;
  unsigned int *map1, *map2, *map3;
  header *h3;
  FILE *outp;
  assert(NULL != inp1);
  assert(NULL != inp2);
  assert(NULL != h1);
  assert(NULL != h2);
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != m3);
  assert(NULL != name);
  nor1 = header_get_nor(h1);
  nor2 = header_get_nor(h2);
  noc2 = header_get_noc(h2);
  map1 = malloc_map(nor1);
  map2 = malloc_map(nor2);
  map3 = malloc_map(nor1);
  if (0 == read_map(inp1, nor1, map1, name, m1) ||
      0 == read_map(inp2, nor2, map2, name, m2) ||
      0 == mul_map(map1, map2, map3, h1, h2, name)) {
    header_free(h1);
    header_free(h2);
    map_free(map1);
    map_free(map2);
    map_free(map3);
    return cleanup(inp1, inp2, NULL);
  }
  header_free(h1);
  header_free(h2);
  fclose(inp1);
  fclose(inp2);
  map_free(map1);
  map_free(map2);
  h3 = header_create(1, 0, 0, noc2, nor1);
  if (0 == open_and_write_binary_header(&outp, h3, m3, name)) {
    fprintf(stderr, "%s: cannot open or write binary header to %s, terminating\n", name, m3);
    return 0;
  }
  if (0 == write_map(outp, nor1, map3, name, m3)) {
    header_free(h3);
    return cleanup(NULL, NULL, outp);
  }
  header_free(h3);
  map_free(map3);
  fclose(outp);
  return 1;
}

static int mul_row_by_map(unsigned int *row_in, unsigned int *row_out, unsigned int *map,
                          unsigned int noc, unsigned int len, unsigned int nob,
                          unsigned int noc_o, prime_opsp operations, const char *m, const char *name)
{
  unsigned int j;
  row_init(row_out, len);
  for (j = 0; j < noc; j++) {
    unsigned int elt = get_element_from_row(nob, j, row_in);
    if (0 != elt) {
      unsigned int k = map[j], elt2;
      if (k >= noc_o) {
        fprintf(stderr, "%s: element %d from map %s out of range (0 - %d), terminating\n", name, k, m, noc_o - 1);
        return 0;
      }
      elt2 = get_element_from_row(nob, k, row_out);
      if (0 != elt2) {
        elt = (*operations->add)(elt, elt2);
      }
      put_element_to_row(nob, k, row_out, elt);
    }
  }
  return 1;
}

static int mul_rows_by_map(unsigned int **row_in, unsigned int **row_out, FILE *inp,
                           unsigned int noc, unsigned int nor, unsigned int len, unsigned int nob,
                           unsigned int noc_o, prime_opsp operations, const char *m, const char *name)
{
  unsigned int i, *map;
  assert(NULL != row_in);
  assert(NULL != row_out);
  assert(NULL != inp);
  assert(NULL != name);
  map = malloc_map(noc);
  if (0 == read_map(inp, noc, map, m, name)) {
    map_free(map);
    return 0;
  }
  for (i = 0; i < nor; i++) {
    if (0 == mul_row_by_map(row_in[i], row_out[i], map, noc, len, nob, noc_o, operations, m, name)) {
      map_free(map);
      return 0;
    }
  }
  map_free(map);
  return 1;
}

static int mul_by_map(FILE *inp1, FILE *inp2, const header *h1, const header *h2,
                      const char *m1, const char *m2, const char *m3, const char *name)
{
  unsigned int nor1, noc1, nor2, noc2, prime, nob, nod, len1, len3, len, i;
  unsigned int *map2, *row1, *row2;
  header *h3;
  FILE *outp;
  prime_ops operations;
  assert(NULL != inp1);
  assert(NULL != inp2);
  assert(NULL != h1);
  assert(NULL != h2);
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != m3);
  assert(NULL != name);
  nor1 = header_get_nor(h1);
  noc1 = header_get_noc(h1);
  nor2 = header_get_nor(h2);
  noc2 = header_get_noc(h2);
  nob = header_get_nob(h1);
  nod = header_get_nod(h1);
  len1 = header_get_len(h1);
  map2 = malloc_map(nor2);
  if (0 == read_map(inp2, nor2, map2, name, m2)) {
    header_free(h1);
    header_free(h2);
    map_free(map2);
    return cleanup(inp1, inp2, NULL);
  }
  prime = header_get_prime(h1);
  header_free(h1);
  header_free(h2);
  fclose(inp2);
  if (noc1 != nor2 ||
      0 == is_a_prime_power(prime)) {
    fprintf(stderr, "%s: %s has bad prime power %d, terminating\n", name, m1, prime);
    map_free(map2);
    return cleanup(inp1, NULL, NULL);
  }
  h3 = header_create(prime, nob, nod, noc2, nor1);
  if (0 == open_and_write_binary_header(&outp, h3, m3, name)) {
    fprintf(stderr, "%s: cannot open or write binary header to %s, terminating\n", name, m3);
    header_free(h3);
    return cleanup(inp1, NULL, NULL);
  }
  len3 = header_get_len(h3);
  header_free(h3);
  len = (len1 > len3) ? len1 : len3;
  if (memory_rows(len, 1000) < 2) {
    fprintf(stderr, "%s: cannot allocate rows for %s * %s, terminating\n", name, m1, m2);
    map_free(map2);
    return cleanup(inp1, NULL, outp);
  }
  row1 = memory_pointer(0);
  row2 = memory_pointer(500);
  if (0 == primes_init(prime, &operations)) {
    fprintf(stderr, "%s: cannot initialise prime operations for %s * %s, terminating\n", name, m1, m2);
    return cleanup(inp1, NULL, outp);
  }
  for (i = 0; i < nor1; i++) {
    if (0 == endian_read_row(inp1, row1, len1)) {
      fprintf(stderr, "%s: cannot read row from %s, terminating\n", name, m1);
      map_free(map2);
      return cleanup(inp1, NULL, outp);
    }
    if (0 == mul_row_by_map(row1, row2, map2, noc1, len3, nob, noc2, &operations, m2, name)) {
      map_free(map2);
      return cleanup(inp1, NULL, outp);
    }
    if (0 == endian_write_row(outp, row2, len3)) {
      fprintf(stderr, "%s: cannot write row to %s, terminating\n", name, m3);
      map_free(map2);
      return cleanup(inp1, NULL, outp);
    }
  }
  map_free(map2);
  fclose(inp1);
  fclose(outp);
  return 1;
}

int mul(const char *m1, const char *m2, const char *m3, const char *name)
{
  FILE *inp1 = NULL;
  FILE *inp2 = NULL;
  FILE *outp = NULL;
  unsigned int prime1, prime2, nob, noc1, nor1, len1, noc2, nor2, len2;
  unsigned int k, j;
  unsigned int nox1, nox2, nox3, nox;
  const header *h1 = NULL, *h2 = NULL, *h3 = NULL;
  unsigned int **rows1, **rows3;
  int is_perm1, is_perm2;
  row_ops row_operations;
  row_adder adder;
  row_incer incer;
  grease_struct grease;
  if (0 == open_and_read_binary_header(&inp1, &h1, m1, name) ||
      0 == open_and_read_binary_header(&inp2, &h2, m2, name)) {
    if (NULL != h1) {
      header_free(h1);
    }
    return cleanup(inp1, inp2, NULL);
  }
  prime1 = header_get_prime(h1);
  prime2 = header_get_prime(h2);
  is_perm1 = (1 == prime1);
  is_perm2 = (1 == prime2);
  nob = header_get_nob(h2);
  nor1 = header_get_nor(h1);
  noc1 = header_get_noc(h1);
  len1 = header_get_len(h1);
  noc2 = header_get_noc(h2);
  nor2 = header_get_nor(h2);
  len2 = header_get_len(h2);
  if (is_perm2) {
    if (is_perm1) {
      return mul_from_maps(inp1, inp2, h1, h2, m1, m2, m3, name);
    } else {
      return mul_by_map(inp1, inp2, h1, h2, m1, m2, m3, name);
    }
  }
  /* We still have the possibility that m1 is a permutation */
  if ((0 == is_perm1 &&
      (header_get_nob(h1) != nob ||
       prime1 != prime2)) ||
      nor2 != noc1) {
    fprintf(stderr, "%s: header mismatch between %s and %s, terminating\n", name, m1, m2);
    header_free(h1);
    header_free(h2);
    return cleanup(inp1, inp2, NULL);
  }
  header_free(h1);
  header_free(h2);
  h3 = header_create(prime2, nob, header_get_nod(h1), noc2, nor1);
  if (0 == open_and_write_binary_header(&outp, h3, m3, name)) {
    fprintf(stderr, "%s: cannot open or write binary header to %s, terminating\n", name, m3);
    header_free(h3);
    return cleanup(inp1, inp2, NULL);
  }
  header_free(h3);
  if (0 == rows_init(prime2, &row_operations)) {
    fprintf(stderr, "%s: cannot initialise row operations for %s, %s, terminating\n", name, m1, m2);
    return cleanup(inp1, inp2, outp);
  }
  grease_init(&row_operations, &grease);
  adder = row_operations.adder;
  incer = row_operations.incer;
  if (is_perm1) {
    len1 = compute_len(nob, noc1);
  }
  nox1 = memory_rows(len1, M1_SIZE);
  nox3 = memory_rows(len2, M1_SIZE); /* len3 = len2 */
  nox = (nox1 > nox3) ? nox3 : nox1; /* Deal with the one with bigger rows */
  nox = (nox > nor1) ? nor1 : nox; /* Only deal with as many rows as we have */
  nox2 = memory_rows(len2, M2_SIZE);
  if (0 == nox || 0 == nox2) {
    fprintf(stderr, "%s: cannot initialise input rows, terminating\n", name);
    return cleanup(inp1, inp2, outp);
  }
  /* Compute best lazy grease given nox2 */
  if (0 == grease_level(prime2, &grease, nox2)) {
    fprintf(stderr, "%s: cannot allocate grease space, terminating\n", name);
    exit(1);
  }
  /* Allocate the grease space */
  if (0 == grease_allocate(prime2, len2, &grease, M1_SIZE)){
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    return cleanup(inp1, inp2, outp);
  }
  rows1 = matrix_malloc(nox);
  rows3 = matrix_malloc(nox);
  for (k = 0; k < nox; k++) {
    rows1[k] = memory_pointer_offset(0, k, len1);
    rows3[k] = memory_pointer_offset(M1_SIZE + M2_SIZE, k, len2);
  }
  for (k = 0; k < nor1; k += nox) {
    unsigned int rest = (nor1 >= k + nox) ? nox : nor1 - k;
    /* Read matrix 1 */
    /* Convert permutation if necessary */
    if (is_perm1) {
      unsigned int *map = malloc_map(rest);
      if (0 == read_map(inp1, rest, map, name, m1)) {
        fprintf(stderr, "%s: unable to read %s, terminating\n", name, m1);
        grease_free(&grease);
        map_free(map);
        return cleanup(inp1, inp2, outp);
      }
      for (j = 0; j < rest; j++) {
        unsigned int l = map[j];
        row_init(rows1[j], len1);
        if (l >= noc1) {
          fprintf(stderr, "%s: map entry %d out of range (0 - %d), terminating\n", name, l, noc1 - 1);
          grease_free(&grease);
          map_free(map);
          return cleanup(inp1, inp2, outp);
        }
        put_element_to_row(nob, l, rows1[j], 1);
      }
      map_free(map);
    } else {
      if (0 == endian_read_matrix(inp1, rows1, len1, rest)) {
        fprintf(stderr, "%s: unable to read %s, terminating\n", name, m1);
        grease_free(&grease);
        return cleanup(inp1, inp2, outp);
      }
    }
    if (0 == mul_from_store(rows1, rows3, inp2, 0, noc1, len2, nob, rest, noc2, prime2, &grease, m2, name)) {
      fclose(inp2);
      fclose(outp);
      matrix_free(rows1);
      matrix_free(rows3);
      grease_free(&grease);
      return 0;
    }
    /* Write matrix 3 */
    if (0 == endian_write_matrix(outp, rows3, len2, rest)) {
      fprintf(stderr, "%s: unable to write %s, terminating\n", name, m3);
      matrix_free(rows1);
      matrix_free(rows3);
      grease_free(&grease);
      return cleanup(inp1, inp2, outp);
    }
  }
  grease_free(&grease);
  matrix_free(rows1);
  matrix_free(rows3);
  fclose(inp1);
  fclose(inp2);
  fclose(outp);
  return 1;
}

#define LAZY_GREASE 1

int mul_from_store(unsigned int **rows1, unsigned int **rows3,
                   FILE *inp, int is_map, unsigned int noc, unsigned int len,
                   unsigned int nob, unsigned int nor, unsigned int noc_o, unsigned int prime,
                   grease grease, const char *m, const char *name)
{
  long pos;
  unsigned int i, j, l;
  row_ops row_operations;
  assert(NULL != inp);
  assert(0 != noc);
  assert(is_a_prime_power(prime)); /* prime refers to rows1 */
  assert(0 != nob); /* nob refers to rows1 */
  /* Remember where we are in row 2 */
  pos = ftell(inp);
  if (is_map) {
    /* Multiply some rows by a map */
    prime_ops operations;
    if (0 == primes_init(prime, &operations)) {
      fprintf(stderr, "%s: cannot initialise prime operations for %s, terminating\n", name, m);
      return 0;
    }
    if (0 == mul_rows_by_map(rows1, rows3, inp, noc, nor, len, nob,
                             noc_o, &operations, m, name)) {
      return 0;
    }
  } else {
    assert(0 != len); /* len may have come from m, and hence would be zero for a map */
    if (0 == rows_init(prime, &row_operations)) {
      fprintf(stderr, "%s: cannot initialise row operations for %s, terminating\n", name, m);
      return 0;
    }
    /* Then multiply */
    for (i = 0; i < noc; i += grease->level) {
      unsigned int size = (grease->level + i <= noc) ? grease->level : noc - i;
      unsigned int word_offset, bit_offset, mask;
      /* Read size rows from matrix 2 into rows 2 */
      /* This sets the initial rows */
      l = 1;
      for (j = 0; j < size; j++) {
        if (0 == endian_read_row(inp, grease->rows[l - 1], len)) {
          fprintf(stderr, "%s: unable to read %s, terminating\n", name, m);
          return 0;
        }
        l *= prime;
      }
      element_access_init(nob, i, size, &word_offset, &bit_offset, &mask);
      grease_init_rows(grease, prime);
#if LAZY_GREASE
#else
      grease_make_rows(grease, size, prime, len);
#endif
      for (j = 0; j < nor; j++) {
        unsigned int *row1 = rows1[j];
        unsigned int elt = get_elements_from_row(row1 + word_offset, size * nob, bit_offset, mask);
        if (0 == i) {
          row_init(rows3[j], len);
        }
        if (0 != elt) {
#if LAZY_GREASE
          grease_row_inc(grease, len, rows3[j], prime, contract(elt, prime, nob));
#else
          (*row_operations.incer)(grease->rows[contract(elt, prime, nob) - 1], rows3[j], len);
#endif
        }
      }
    }
  }
  /* Move back in matrix 2 */
  if (0 != fseek(inp, pos, SEEK_SET)) {
    fprintf(stderr, "%s: unable to rewind %s, terminating\n", name, m);
    return 0;
  }
  return 1;
}
