/*
 * $Id: ident.c,v 1.6 2002/01/06 16:35:48 jon Exp $
 *
 * Subroutine to generate identity matrix
 *
 */

#include "ident.h"
#include <stdio.h>
#include <assert.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "primes.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

int ident(unsigned int prime, unsigned int nor, unsigned int noc,
          const char *out, const char *name)
{
  unsigned int nob, nod, len;
  unsigned int i;
  unsigned int *row;
  FILE *outp;
  const header *h;

  assert(NULL != out);
  assert(NULL != name);
  if (0 == is_a_prime_power(prime)) {
    fprintf(stderr, "%s: non prime %d\n", name, prime);
    return 0;
  }
  nob = bits_of(prime);
  nod = digits_of(prime);
  h = header_create(prime, nob, nod, noc, nor);
  assert(NULL != h);
  len = header_get_len(h);
  assert(0 != len);
  if (0 == open_and_write_binary_header(&outp, h, out, name)) {
    fprintf(stderr, "%s: cannot open or write header to %s\n", name, out);
    header_free(h);
    return 0;
  }
  header_free(h);
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s: cannot create output row\n", name);
    fclose(outp);
    return 0;
  }
  row = memory_pointer_offset(0, 0, len);
  assert(NULL != row);
  for (i = 0; i < nor; i++) {
    row_init(row, len);
    if (i < noc) {
      put_element_to_row(nob, i, row, 1);
    }
    if (0 == endian_write_row(outp, row, len)) {
      fprintf(stderr, "%s: write output row to %s\n", name, out);
      fclose(outp);
      return 0;
    }
  }
  fclose(outp);
  return 1;
}
