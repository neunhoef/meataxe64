/*
 * $Id: powers.c,v 1.1 2002/02/05 19:50:56 jon Exp $
 *
 * Function to compute tensor powers of a matrix, from file
 *
 */

#include "powers.h"
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "write.h"
#include <stdio.h>
#include <assert.h>

static int cleanup(FILE *inp, FILE *outp)
{
  if (NULL != inp)
    fclose(inp);
  if (NULL != outp)
    fclose(outp);
  return 0;
}

int skew_square(const char *m1, const char *m2, const char *name)
{
  FILE *inp = NULL;
  FILE *outp = NULL;
  unsigned int prime, nob, nod, nor_in, len_in, nor_out, len_out;
  const header *h_in = NULL, *h_out = NULL;
  unsigned int i, j, k, l;
  unsigned int **rows, *row_out;
  prime_ops prime_operations;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp, &h_in, m1, name)) {
    if (NULL != h_in) {
      header_free(h_in);
    }
    return cleanup(inp, outp);
  }
  prime = header_get_prime(h_in);
  nob = header_get_nob(h_in);
  nod = header_get_nod(h_in);
  nor_in = header_get_nor(h_in);
  len_in = header_get_len(h_in);
  if (nor_in != header_get_noc(h_in)) {
    fprintf(stderr, "%s: %s is not square, terminating\n", name, m1);
    header_free(h_in);
    return cleanup(inp, NULL);
  }
  header_free(h_in);
  if (0 == primes_init(prime, &prime_operations)) {
    fprintf(stderr, "%s: Can't initialise prime operations for %s, terminating\n", name, m1);
    return cleanup(inp, NULL);
  }
  nor_out = (nor_in * (nor_in - 1)) / 2;
  h_out = header_create(prime, nob, nod, nor_out, nor_out);
  len_out = header_get_len(h_out);
  if (memory_rows(len_in, 900) < nor_in ||
      memory_rows(len_out, 100) < 1) {
    fprintf(stderr, "%s: cannot get enough memory, terminating\n", name);
    (void) cleanup(inp, outp);
    exit(2);
  }
  rows = matrix_malloc(nor_in);
  for (i = 0; i < nor_in; i++) {
    rows[i] = memory_pointer_offset(0, i, len_in);
  }
  row_out = memory_pointer(900);
  if (0 == endian_read_matrix(inp, rows, len_in, nor_in)) {
    fprintf(stderr, "%s: cannot read some of %s, terminating\n", name, m1);
    return cleanup(inp, outp);
  }
  if (0 == open_and_write_binary_header(&outp, h_out, m2, name)) {
    return cleanup(inp, outp);
  }
  fclose(inp);
  header_free(h_out);
  for (i = 0; i + 1 < nor_in; i++) {
    /* Down the rows of m1 */
    for (j = i + 1; j < nor_in; j++) {
      /* Down the rows of m1 again */
      unsigned int offset = 0;
      row_init(row_out, len_out);
      for (k = 0; k + 1 < nor_in; k++) {
        /* Along the columns of m1 */
        unsigned int elt1 = get_element_from_row(nob, k, rows[i]);
        unsigned int elt2 = get_element_from_row(nob, k, rows[j]);
        for (l = k + 1; l < nor_in; l++) {
          /* Along the columns of m1 again */
          unsigned int elt3 = get_element_from_row(nob, l, rows[i]);
          unsigned int elt4 = get_element_from_row(nob, l, rows[j]);
          unsigned int e1 = (*prime_operations.mul)(elt1, elt4);
          unsigned int e2 = (*prime_operations.negate)((*prime_operations.mul)(elt2, elt3));
          unsigned int e = (*prime_operations.add)(e1, e2);
          put_element_to_row(nob, offset, row_out, e);
          offset++;
        }
      }
      assert(offset == nor_out);
      if (0 == endian_write_row(outp, row_out, len_out)) {
        fprintf(stderr, "%s: cannot write some of %s, terminating\n", name, m2);
        fclose(outp);
        return 0;
      }
    }
  }
  matrix_free(rows);
  fclose(outp);
  return 1;
}
