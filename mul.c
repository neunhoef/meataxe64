/*
 * $Id: mul.c,v 1.18 2001/11/29 01:13:09 jon Exp $
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
  if (NULL != inp1)
    fclose(inp2);
  if (NULL != inp1)
    fclose(outp);
  return 0;
}

int mul(const char *m1, const char *m2, const char *m3, const char *name)
{
  FILE *inp1 = NULL;
  FILE *inp2 = NULL;
  FILE *outp = NULL;
  unsigned int prime, nob, noc1, nor1, len1, noc2, nor2, len2;
  unsigned int k;
  unsigned int nox1, nox2, nox3, nox;
  const header *h1 = NULL, *h2 = NULL, *h3 = NULL;
  unsigned int **rows1, **rows3;
  row_ops row_operations;
  row_adder adder;
  row_incer incer;
  grease_struct grease;
  endian_init();
  if (0 == open_and_read_binary_header(&inp1, &h1, m1, name) ||
      0 == open_and_read_binary_header(&inp2, &h2, m2, name)) {
    if (NULL != h1) {
      header_free(h1);
    }
    if (NULL != h2) {
      header_free(h2);
    }
    return cleanup(inp1, inp2, NULL);
  }
  prime = header_get_prime(h1);
  nob = header_get_nob(h1);
  nor1 = header_get_nor(h1);
  noc1 = header_get_noc(h1);
  len1 = header_get_len(h1);
  noc2 = header_get_noc(h2);
  nor2 = header_get_nor(h2);
  len2 = header_get_len(h2);
  if (header_get_prime(h2) != prime ||
      header_get_nob(h2) != nob ||
      nor2 != noc1) {
    fprintf(stderr, "%s: header mismatch between %s and %s, terminating\n", name, m1, m2);
    header_free(h1);
    header_free(h2);
    return cleanup(inp1, inp2, NULL);
  }
  h3 = header_create(prime, nob, header_get_nod(h1), noc2, nor1);
  if (0 == open_and_write_binary_header(&outp, h3, m3, name)) {
    fprintf(stderr, "%s: cannot open or write binary header to %s, terminating\n", name, m3);
    header_free(h1);
    header_free(h2);
    header_free(h3);
    return cleanup(inp1, inp2, NULL);
  }
  header_free(h1);
  header_free(h2);
  header_free(h3);
  if (0 == rows_init(prime, &row_operations)) {
    fprintf(stderr, "%s: cannot initialise row operations for %s, %s, terminating\n", name, m1, m2);
    return cleanup(inp1, inp2, outp);
  }
  grease_init(&row_operations, &grease);
  adder = row_operations.adder;
  incer = row_operations.incer;
  nox1 = memory_rows(len1, M1_SIZE);
  nox3 = memory_rows(len2, M1_SIZE); /* len3 = len2 */
  nox = (nox1 > nox3) ? nox3 : nox1; /* Deal with the one with bigger rows */
  nox = (nox > nor1) ? nor1 : nox; /* Only deal with as many rows as we have */
  nox2 = memory_rows(len2, M2_SIZE);
  if (0 == nox || 0 == nox2) {
    fprintf(stderr, "%s: cannot initialise input rows, terminating\n", name);
    return cleanup(inp1, inp2, outp);
  }
  printf("mul handling %d rows from %d at a time\n", nox, nor1);
  printf("mul nox2 = %d rows\n", nox2);
  /* Compute best lazy grease given nox2 */
  if (0 == grease_level(prime, &grease, nox2)) {
    fprintf(stderr, "%s: cannot allocate grease space, terminating\n", name);
    exit(1);
  }
  /* Allocate the grease space */
  if (0 == grease_allocate(prime, len2, &grease, M1_SIZE)){
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
    if (0 == endian_read_matrix(inp1, rows1, len1, rest)) {
      fprintf(stderr, "%s: unable to read %s, terminating\n", name, m1);
      return cleanup(inp1, inp2, outp);
    }
    if (0 == mul_from_store(rows1, rows3, inp2, noc1, len2, nob, rest, prime, &grease, m2, name)) {
      fclose(inp2);
      fclose(outp);
      return 0;
    }
    /* Write matrix 3 */
    if (0 == endian_write_matrix(outp, rows3, len2, rest)) {
      fprintf(stderr, "%s: unable to write %s, terminating\n", name, m3);
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

int mul_from_store(unsigned int **rows1, unsigned int **rows3,
                   FILE *inp, unsigned int noc, unsigned int len,
                   unsigned int nob, unsigned int nor, unsigned int prime,
                   grease grease, const char *m, const char *name)
{
  long pos;
  unsigned int i, j, l;
  row_ops row_operations;
  assert(NULL != inp);
  assert(is_a_prime_power(prime));
  assert(0 != nob);
  assert(0 != len);
  assert(0 != noc);
  if (0 == rows_init(prime, &row_operations)) {
    fprintf(stderr, "%s: cannot initialise row operations for %s, terminating\n", name, m);
    fclose(inp);
    return 0;
  }
  /* Remember where we are in row 2 */
  pos = ftell(inp);
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
        fclose(inp);
        return 0;
      }
      l *= prime;
    }
    element_access_init(nob, i, size, &word_offset, &bit_offset, &mask);
    grease_init_rows(grease, prime);
    if (0 == grease_make_rows(grease, size, prime, len)) {
      fprintf(stderr, "%s: unable to compute grease, terminating\n", name);
      fclose(inp);
      return 0;
    }
    for (j = 0; j < nor; j++) {
      unsigned int *row1 = rows1[j];
      unsigned int elt = get_elements_from_row(row1 + word_offset, size * nob, bit_offset, mask);
      if (0 == i) {
        row_init(rows3[j], len);
      }
      if (0 != elt) {
        (*row_operations.incer)(grease->rows[contract(elt, prime, nob) - 1], rows3[j], len);
      }
    }
  }
  /* Move back in matrix 2 */
  if (0 != fseek(inp, pos, SEEK_SET)) {
    fprintf(stderr, "%s: unable to rewind %s, terminating\n", name, m);
    fclose(inp);
    return 0;
  }
  return 1;
}
