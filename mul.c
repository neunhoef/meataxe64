/*
 * $Id: mul.c,v 1.6 2001/09/13 19:44:31 jon Exp $
 *
 * Function to multiply two matrices to give a third
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "header.h"
#include "utils.h"
#include "read.h"
#include "write.h"
#include "elements.h"
#include "endian.h"
#include "rows.h"
#include "grease.h"
#include "matrix.h"
#include "mul.h"

int mul(const char *m1, const char *m2, const char *m3, const char *name)
{
  FILE *inp1;
  FILE *inp2;
  FILE *outp;
  unsigned int prime, nob, noc1, nor1, len1, noc2, nor2, len2;
  unsigned int i, j;
  unsigned int step;
  const header *h1, *h2, *h3;
  unsigned int **rows1, **rows2, **rows3;
  unsigned int grease_row_count;
  unsigned int ** grease_rows;
  row_ops row_operations;
  row_adder adder;
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
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
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
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
  }
  h3 = header_create(prime, nob, header_get_nod(h1), noc2, nor1);
  if (0 == write_binary_header(outp, h3, m3)) {
    fprintf(stderr, "%s: cannot write binary header to %s, terminating\n", name, m3);
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
  }
  if (0 == rows_init(prime, &row_operations)) {
    fprintf(stderr, "%s: cannot initialise row operations for %s, %s, terminating\n", name, m1, m2);
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
  }
  adder = row_operations.adder;
  scaled_adder = row_operations.scaled_adder;
  if (0 == grease(nob, nor1, noc1, noc2, prime, &step)) {
    fprintf(stderr, "%s: cannot compute grease level for %s, %s, terminating\n", name, m1, m2);
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
  }
  /* For the moment, read entire matrices for 1 & 3 */
  if (0 == matrix_malloc(len1, nor1, &rows1) ||
      0 == matrix_malloc(len2, nor1, &rows3)) {
    fprintf(stderr, "%s: cannot allocate rows for %s, %s, %s, terminating\n", name, m1, m2, m3);
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
  }
  /* Here is where to insert loop to read only partial matrices */
  /* Read matrix 1 */
  if (0 == endian_read_matrix(inp1, rows1, len1, nor1)) {
    fprintf(stderr, "%s: unable to read %s, terminating\n", name, m1);
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
  }
  /* Allocate the grease space */
  if (step > 1 && 0 == grease_allocate_rows(step, prime, len2,
                                            &grease_row_count, &grease_rows)) {
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
  } /* Could consider working ungreased */
  /* Allocate step rows for input 2 */
  if (0 == matrix_malloc(len2, step, &rows2)) {
    fprintf(stderr, "%s: cannot allocate rows for %s, %s, %s, terminating\n", name, m1, m2, m3);
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
  }
  /* Then multiply */
  for (i = 0; i < noc1; i += step) {
    unsigned int size = (step + i <= noc1) ? step : noc1 - i;
    unsigned int word_offset, bit_offset, mask;
    /* Read step rows from matrix 2 into rows 2 */
    if (0 == endian_read_matrix(inp2, rows2, len2, step)) {
      fprintf(stderr, "%s: unable to read %s, terminating\n", name, m2);
      fclose(inp1);
      fclose(inp2);
      fclose(outp);
      return 0;
    }
    element_access_init(nob, i, size, &word_offset, &bit_offset, &mask);
    if (step > 1 && 0 == grease_make_rows(rows2, size, prime, len2, 0, &grease_rows))
    {
      fprintf(stderr, "%s: unable to compute grease, terminating\n", name);
      fclose(inp1);
      fclose(inp2);
      fclose(outp);
      return 0;
    }
    for (j = 0; j < nor1; j++) {
      unsigned int *row1 = rows1[j];
      unsigned int elt = get_elements_from_row(row1 + word_offset, bit_offset, mask);
      if (0 == i) {
        row_init(rows3[j], len2);
      }
      if (0 != elt) {
        int res;
        if (step > 1) {
          res = (*adder)(rows3[j], grease_rows[elt-1], rows3[j], len2);
        } else {
          res = (1 == elt) ? (*adder)(rows3[j], rows2[0], rows3[j], len2) :
              (*scaled_adder)(rows3[j], rows2[0], rows3[j], len2, elt);
        }
        if (0 == res) {
          fprintf(stderr, "%s: add failed, terminating\n", name);
          fclose(inp1);
          fclose(inp2);
          fclose(outp);
          return 0;
        }
      }
    }
  }
  /* Write matrix 3 */
  if (0 == endian_write_matrix(outp, rows3, len2, nor1)) {
    fprintf(stderr, "%s: unable to write %s, terminating\n", name, m3);
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
  }
  fclose(inp1);
  fclose(inp2);
  fclose(outp);
  return 1;
}
