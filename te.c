/*
 * $Id: te.c,v 1.4 2002/02/05 19:50:56 jon Exp $
 *
 * Function to tensor two matrices to give a third
 *
 */

#include "te.h"
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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

int tensor(const char *m1, const char *m2, const char *m3, const char *name)
{
  FILE *inp1 = NULL;
  FILE *inp2 = NULL;
  FILE *outp = NULL;
  unsigned int prime, nob, noc1, nor1, len1, noc2, nor2, len2, noc3, nor3, len3;
  const header *h1 = NULL, *h2 = NULL, *h3 = NULL;
  unsigned int i, j, k, l;
  unsigned int **rows1, **rows2, *row_out, *row_copy;
  row_ops row_operations;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != m3);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp1, &h1, m1, name) ||
      0 == open_and_read_binary_header(&inp2, &h2, m2, name)) {
    if (NULL != h1) {
      header_free(h1);
    }
    if (NULL != h2) {
      header_free(h2);
    }
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
      nor1 != noc1 ||
      nor2 != noc2) {
    fprintf(stderr, "%s: header mismatch between %s and %s, terminating\n", name, m1, m2);
    header_free(h1);
    header_free(h2);
    return cleanup(inp1, inp2, NULL);
  }
  nor3 = nor1 * nor2;
  noc3 = nor3;
  h3 = header_create(prime, nob, header_get_nod(h1), noc3, nor3);
  len3 = header_get_len(h3);
  if (memory_rows(len1, 450) < nor1 ||
      memory_rows(len2, 450) < nor2 ||
      memory_rows(len3, 100) < 2) {
    fprintf(stderr, "%s: cannot get enough memory, terminating\n", name);
    (void) cleanup(inp1, inp2, outp);
    exit(2);
  }
  rows1 = matrix_malloc(nor1);
  rows2 = matrix_malloc(nor2);
  for (i = 0; i < nor1; i++) {
    rows1[i] = memory_pointer_offset(0, i, len1);
  }
  for (i = 0; i < nor2; i++) {
    rows2[i] = memory_pointer_offset(450, i, len2);
  }
  row_out = memory_pointer(900);
  row_copy = memory_pointer(950);
  if (0 == endian_read_matrix(inp1, rows1, len1, nor1) ||
      0 == endian_read_matrix(inp2, rows2, len2, nor2)) {
    fprintf(stderr, "%s: cannot read some of %s or %s, terminating\n", name, m1, m2);
    return cleanup(inp1, inp2, outp);
  }
  if (0 == open_and_write_binary_header(&outp, h3, m3, name)) {
    return cleanup(inp1, inp2, outp);
  }
  fclose(inp1);
  fclose(inp2);
  header_free(h1);
  header_free(h2);
  header_free(h3);
  rows_init(prime, &row_operations);
  for (i = 0; i < nor1; i++) {
    /* Down the rows of m1 */
    for (j = 0; j < nor2; j++) {
      /* Down the rows of m2 */
      unsigned int offset = 0;
      row_init(row_out, len3);
      for (k = 0; k < noc1; k++) {
        /* Along the columns of m1 */
        unsigned int elt = get_element_from_row(nob, k, rows1[i]);
        if ( 0 != elt) {
          unsigned int *row = rows2[j];
          if (1 != elt) {
            (*row_operations.scaler)(row, row_copy, len2, elt);
            row = row_copy;
          }
          for (l = 0; l < noc2; l++) {
            elt = get_element_from_row(nob, l, row);
            put_element_to_row(nob, offset + l, row_out, elt);
          }
        }
        offset += noc2;
      }
      if (0 == endian_write_row(outp, row_out, len3)) {
        fprintf(stderr, "%s: cannot write some of %s, terminating\n", name, m3);
        fclose(outp);
        return 0;
      }
    }
  }
  matrix_free(rows1);
  matrix_free(rows2);
  fclose(outp);
  return 1;
}
