/*
 * $Id: restrict.c,v 1.1 2002/03/20 18:42:30 jon Exp $
 *
 * Function to restrict a matrix from a big field to a smaller
 *
 */

#include "restrict.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

static int cleanup(FILE *inp, FILE *outp)
{
  if (NULL != inp) {
    fclose(inp);
  }
  if (NULL != outp) {
    fclose(outp);
  }
  return 0;
}

int restrict(const char *m1, const char *m2, unsigned int q, const char *name)
{
  FILE *inp = NULL;
  FILE *outp = NULL;
  unsigned int prime_in, nob_in, noc, nor, len_in,
      nob_out, nod_out, len_out, base_prime, index_in, index_out;
  unsigned int i;
  const header *h_in;
  header *h_out;
  unsigned int *row1, *row2;

  if (0 == open_and_read_binary_header(&inp, &h_in, m1, name)) {
    return 0;
  }
  prime_in = header_get_prime(h_in);
  nob_in = header_get_nob(h_in);
  nor = header_get_nor(h_in);
  noc = header_get_noc(h_in);
  len_in = header_get_len(h_in);
  base_prime = prime_divisor(prime_in);
  index_in = prime_index(prime_in, base_prime);
  header_free(h_in);
  if (0 != prime_in % q) {
    fprintf(stderr, "%s: %d does not divide %d\n", name, q, prime_in);
    header_free(h_in);
    return cleanup(inp, NULL);
  }
  index_out = prime_index(q, base_prime);
  if (0 != index_in % index_out) {
    fprintf(stderr, "%s: field of order %d does not divide is not a subfield of field of order %d\n",
            name, q, prime_in);
    header_free(h_in);
    return cleanup(inp, NULL);
  }
  nob_out = bits_of(q);
  nod_out = digits_of(q);
  h_out = header_create(q, nob_out, nod_out, noc, nor);
  len_out = header_get_len(h_out);
  if (0 == open_and_write_binary_header(&outp, h_out, m2, name)) {
    header_free(h_in);
    return cleanup(inp, NULL);
  }
  header_free(h_out);
  if (memory_rows(len_in, 1000) < 2) {
    fprintf(stderr, "%s: cannot allocate rows for %s, %s, terminating\n", name, m1, m2);
    return cleanup(inp, outp);
  }
  row1 = memory_pointer_offset(0, 0, len_in);
  row2 = memory_pointer_offset(0, 1, len_in);
  for (i = 0; i < nor; i++) {
    unsigned int j;
    if (0 == endian_read_row(inp, row1, len_in)) {
      fprintf(stderr, "%s: cannot read row %d from %s, terminating\n", name, i, m1);
      return cleanup(inp, outp);
    }
    row_init(row2, len_out);
    for (j = 0; j < noc; j++) {
      unsigned int elt = get_element_from_row(nob_in, j, row1);
      if (elt >= q) {
        fprintf(stderr, "%s: element %d from row %d, column %d of %s is not in field of order %d, terminating\n", name, elt, i, j, m1, q);
        (void)cleanup(inp, outp);
        return 255;
      }
      put_element_to_row(nob_out, j, row2, elt);
    }
    if (0 == endian_write_row(outp, row2, len_out)) {
      fprintf(stderr, "%s: cannot write row %d to %s, terminating\n", name, i, m2);
      return cleanup(inp, outp);
    }
  }
  (void)cleanup(inp, outp);
  return 1;
}
