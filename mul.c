/*
 * $Id: mul.c,v 1.2 2001/09/02 22:16:41 jon Exp $
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
  write_binary_header(outp, h3, m3);
  if (0 == grease(nob, nor1, noc1, noc2, prime, &step)) {
    fprintf(stderr, "%s: cannot compute grease for %s, %s, terminating\n", name, m1, m2);
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
  /* Then multiply */
  for (i = 0; i < noc1; i += step) {
    unsigned int grease_row_count;
    unsigned int size = (step + i <= noc1) ? step : noc1 - i;
    unsigned int **grease_rows = grease_make_rows(rows2, size, prime, nob, i, &grease_row_count);
    /* Initialise the grease for matrix 2 */
    for (j = 0; j < nor1; j++) {
      unsigned int *row1 = rows1[j];
      unsigned int elt = grease_get_elt(row1, i, size, prime, nob);
      if (0 == i) {
        row_init(rows3[j], nob, noc2);
      }
      if (0 != elt) {
        unsigned int *grease_row = grease_rows[elt-1];
        if (0 == row_add(rows3[j], grease_row, rows3[j], prime, nob, noc2)) {
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
