/*
 * $Id: mul.c,v 1.3 2001/09/04 23:00:12 jon Exp $
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
  unsigned int prime, nob, noc1, nor1, noc2, nor2;
  unsigned int i, j;
  unsigned int step;
  header h1, h2, h3;
  unsigned int **rows1, **rows2, **rows3;
  unsigned int grease_row_count;
  unsigned int ** grease_rows;
  row_ops row_operations;
  row_adder adder;

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
  noc2 = header_get_noc(h2);
  nor2 = header_get_nor(h2);
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
  if (0 == grease(nob, nor1, noc1, noc2, prime, &step)) {
    fprintf(stderr, "%s: cannot compute grease level for %s, %s, terminating\n", name, m1, m2);
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
  }
  /* Here is where to insert loop to read only partial matrices */
  /* Initialise grease, deferred */
  /* Sort out prime operations structs */
  /* For the moment, read entire matrices */
  if (0 == matrix_malloc(nob, nor1, noc1, &rows1) ||
      0 == matrix_malloc(nob, nor2, noc2, &rows2) ||
      0 == matrix_malloc(nob, nor1, noc2, &rows3)) {
    fprintf(stderr, "%s: cannot allocate rows for %s, %s, %s, terminating\n", name, m1, m2, m3);
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
  }
  /* Read matrix 1 */
  if (0 == endian_read_matrix(inp1, rows1, nob, noc1, nor2)) {
    fprintf(stderr, "%s: unable to read %s, terminating\n", name, m1);
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
  }
  /* Read matrix 2 */
  if (0 == endian_read_matrix(inp2, rows2, nob, noc2, nor2)) {
    fprintf(stderr, "%s: unable to read %s, terminating\n", name, m2);
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
  }
  /* Allocate the grease space */
  if (0 == grease_allocate_rows(step, prime, nob, noc2,
                                &grease_row_count, &grease_rows)) {
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
  } /* Could consider working ungreased */
  /* Then multiply */
  for (i = 0; i < noc1; i += step) {
    unsigned int size = (step + i <= noc1) ? step : noc1 - i;
    unsigned int word_offset, bit_offset, mask;
    element_access_init(nob, i, size, &word_offset, &bit_offset, &mask);
    if (0 == grease_make_rows(rows2, size, prime, noc2, i, &grease_rows))
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
        row_init(rows3[j], nob, noc2);
      }
      if (0 != elt) {
        unsigned int *grease_row = grease_rows[elt-1];
        if (0 == (*adder)(rows3[j], grease_row, rows3[j], noc2)) {
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
  if (0 == endian_write_matrix(outp, rows3, nob, noc2, nor1)) {
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
