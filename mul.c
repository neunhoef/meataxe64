/*
 * $Id: mul.c,v 1.9 2001/09/20 00:00:16 jon Exp $
 *
 * Function to multiply two matrices to give a third
 *
 */

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
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"
#include "mul.h"

#define M1_SIZE 450
#define M2_SIZE 100

static int cleanup(FILE *inp1, FILE *inp2, FILE *outp)
{
  fclose(inp1);
  fclose(inp2);
  fclose(outp);
  return 0;
}

int mul(const char *m1, const char *m2, const char *m3, const char *name)
{
  FILE *inp1;
  FILE *inp2;
  FILE *outp;
  unsigned int prime, nob, noc1, nor1, len1, noc2, nor2, len2;
  unsigned int i, j, k, l;
  unsigned int step;
  unsigned int nox1, nox2, nox3, nox;
  void *t1, *t2;
  const header *h1, *h2, *h3;
  unsigned int **rows1, **rows3;
  unsigned int **grease_rows;
  int greased = 1;
  row_ops row_operations;
  row_adder adder;
  row_incer incer;
  scaled_row_adder scaled_adder;
  endian_init();
  inp1 = fopen(m1, "rb");
  if (NULL == inp1) {
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, m1);
    return 0;
  }
  inp2 = fopen(m2, "rb");
  if (NULL == inp2) {
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, m2);
    fclose(inp1);
    return 0;
  }
  outp = fopen(m3, "wb");
  if (NULL == outp) {
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, m3);
    fclose(inp1);
    fclose(inp2);
    return 0;
  }
  if (0 == read_binary_header(inp1, &h1, m1) ||
      0 == read_binary_header(inp2, &h2, m2)) {
    return cleanup(inp1, inp2, outp);
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
    return cleanup(inp1, inp2, outp);
  }
  h3 = header_create(prime, nob, header_get_nod(h1), noc2, nor1);
  if (0 == write_binary_header(outp, h3, m3)) {
    fprintf(stderr, "%s: cannot write binary header to %s, terminating\n", name, m3);
    return cleanup(inp1, inp2, outp);
  }
  if (0 == rows_init(prime, &row_operations)) {
    fprintf(stderr, "%s: cannot initialise row operations for %s, %s, terminating\n", name, m1, m2);
    return cleanup(inp1, inp2, outp);
  }
  grease_init(&row_operations);
  adder = row_operations.adder;
  incer = row_operations.incer;
  scaled_adder = row_operations.scaled_adder;
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
  /* Compute best lazy grease give nox2 */
  if (0 == grease(prime, &step, nox2)) {
      greased = 0;
  }
  /* Allocate the grease space */
  if (greased) {
    if (0 == grease_allocate_rows(step, prime, len2, &grease_rows, M1_SIZE)){
      fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
      return cleanup(inp1, inp2, outp);
    }
  } else {
    if (0 == matrix_malloc(1, &t1)) {
      fprintf(stderr, "%s: unable to allocate row for matrix 2, terminating\n", name);
      return cleanup(inp1, inp2, outp);
    }
    grease_rows = t1;
    *grease_rows = memory_pointer(M1_SIZE);
  }
/* Could consider working ungreased */
  if (0 == matrix_malloc(nox, &t1) ||
      0 == matrix_malloc(nox, &t2)) {
    fprintf(stderr, "%s: cannot allocate rows for %s, %s, %s, terminating\n", name, m1, m2, m3);
    return cleanup(inp1, inp2, outp);
  }
  /* Set up pointers for rows1 and rows3 */
  rows1 = t1;
  rows3 = t2;
  for (k = 0; k < nox; k++) {
    rows1[k] = memory_pointer_offset(0, k, len1);
    rows3[k] = memory_pointer_offset(M1_SIZE + M2_SIZE, k, len2);
  }
  for (k = 0; k < nor1; k += nox) {
    unsigned int rest = (nor1 >= k + nox) ? nox : nor1 - k;
    long pos;
    /* Read matrix 1 */
    if (0 == endian_read_matrix(inp1, rows1, len1, rest)) {
      fprintf(stderr, "%s: unable to read %s, terminating\n", name, m1);
      return cleanup(inp1, inp2, outp);
    }
    /* Remember where we are in row 2 */
    pos = ftell(inp2);
    /* Then multiply */
    for (i = 0; i < noc1; i += step) {
      unsigned int size = (step + i <= noc1) ? step : noc1 - i;
      unsigned int word_offset, bit_offset, mask;
      /* Read size rows from matrix 2 into rows 2 */
      l = 1;
      for (j = 0; j < size; j++) {
        if (0 == endian_read_row(inp2, grease_rows[l - 1], len2)) {
          fprintf(stderr, "%s: unable to read %s, terminating\n", name, m2);
          return cleanup(inp1, inp2, outp);
        }
        l *= prime;
      }
      element_access_init(nob, i, size, &word_offset, &bit_offset, &mask);
      grease_init_rows(size, prime);
      if (0 == grease_make_rows(size, prime, len2)) {
        fprintf(stderr, "%s: unable to compute grease, terminating\n", name);
        return cleanup(inp1, inp2, outp);
      }
      for (j = 0; j < rest; j++) {
        unsigned int *row1 = rows1[j];
        unsigned int elt = get_elements_from_row(row1 + word_offset, size * nob, bit_offset, mask);
        if (0 == i) {
          row_init(rows3[j], len2);
        }
        if (0 != elt) {
          int res;
          if (greased) {
            res = (*incer)(grease_rows[elements_contract(elt, prime, nob)-1], rows3[j], len2);
          } else {
            res = (1 == elt) ? (*incer)(grease_rows[0], rows3[j], len2) :
                (*scaled_adder)(rows3[j], grease_rows[0], rows3[j], len2, elt);
          }
          if (0 == res) {
            fprintf(stderr, "%s: add failed, terminating\n", name);
            return cleanup(inp1, inp2, outp);
          }
        }
      }
    }
    /* Move back in matrix 2 */
    if (0 != fseek(inp2, pos, SEEK_SET)) {
      fprintf(stderr, "%s: unable to rewind %s, terminating\n", name, m2);
      return cleanup(inp1, inp2, outp);
    }
    /* Write matrix 3 */
    if (0 == endian_write_matrix(outp, rows3, len2, rest)) {
      fprintf(stderr, "%s: unable to write %s, terminating\n", name, m3);
      return cleanup(inp1, inp2, outp);
    }
  }
  if (greased)
    grease_free_rows(grease_rows);
  matrix_free(rows1);
  matrix_free(rows3);
  fclose(inp1);
  fclose(inp2);
  fclose(outp);
  return 1;
}
