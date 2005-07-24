/*
 * $Id: rand.c,v 1.5 2005/07/24 09:32:45 jon Exp $
 *
 * Subroutine to generate a random matrix
 *
 */

#include "rand.h"
#include <stdio.h>
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

int random(u32 prime, u32 nor, u32 noc,
           const char *out, const char *name)
{
  u32 nob, nod, len, i, elts_per_word;
  word *row;
  FILE *outp;
  const header *h;

  assert(NULL != out);
  assert(NULL != name);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot produce random maps\n", name);
    return 0;
  } else {
    if (0 == is_a_prime_power(prime)) {
      fprintf(stderr, "%s: non prime %u\n", name, prime);
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
    (void)get_mask_and_elts(nob, &elts_per_word);
    for (i = 0; i < nor; i++) {
      u32 j;
      row_init(row, len);
      for (j = 0; j < noc; j++) {
        u32 k = rand() % prime;
        put_element_to_clean_row_with_params(nob, j, elts_per_word, row, k);
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
