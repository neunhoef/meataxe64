/*
 * $Id: rand.c,v 1.1 2002/07/20 12:55:21 jon Exp $
 *
 * Subroutine to generate a random matrix
 *
 */

#include "rand.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "maps.h"
#include "memory.h"
#include "primes.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

int random(unsigned int prime, unsigned int nor, unsigned int noc,
           const char *out, const char *name)
{
  unsigned int nob, nod, len;
  unsigned int i;
  unsigned int *row;
  FILE *outp;
  const header *h;

  assert(NULL != out);
  assert(NULL != name);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot produce random maps\n", name);
    return 0;
  } else {
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
      unsigned int j;
      row_init(row, len);
      for (j = 0; j < noc; j++) {
        unsigned int k = rand() % prime;
        put_element_to_row(nob, j, row, k);
      }
      errno = 0;
      if (0 == endian_write_row(outp, row, len)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot write output row to %s\n", name, out);
        fclose(outp);
        return 0;
      }
    }
  }
  fclose(outp);
  return 1;
}
