/*
 * $Id: ident.c,v 1.1 2001/10/10 23:19:42 jon Exp $
 *
 * Subroutine to generate identity matrix
 *
 */

#include <stdio.h>
#include <assert.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "rows.h"
#include "utils.h"
#include "write.h"
#include "ident.h"

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
  endian_init();
  outp = fopen(out, "wb");
  if (NULL == outp) {
    fprintf(stderr, "%s: cannot open %s\n", name, out);
    return 0;
  }
  h = header_create(prime, nob, nod, noc, nor);
  assert(NULL != h);
  len = header_get_len(h);
  assert(0 != len);
  if (0 == write_binary_header(outp, h, out)) {
    fprintf(stderr, "%s: cannot write header\n", name);
    fclose(outp);
    return 0;
  }
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
