/*
 * $Id: extend.c,v 1.4 2003/04/18 22:21:41 jon Exp $
 *
 * Function to extend the field of a row
 *
 */

#include "extend.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "elements.h"
#include "primes.h"

int extend(const unsigned int *in, unsigned int *out,
           unsigned int in_nob, unsigned int out_nob,
           unsigned int in_prime, unsigned int out_prime,
           unsigned int noc, const char *name)
{
  unsigned int i, in_mask, out_mask, elts_per_in_word, elts_per_out_word;
  assert(NULL != in);
  assert(NULL != out);
  if (0 == is_a_prime(in_prime) || 0 == is_a_prime_power(out_prime) || 0 != out_prime % in_prime) {
    fprintf(stderr, "%s: bad prime %d or prime power %d, terminating\n", name, in_prime, out_prime);
    return 0;
  }
  in_mask = get_mask_and_elts(in_nob, &elts_per_in_word);
  out_mask = get_mask_and_elts(out_nob, &elts_per_out_word);
  for (i = 0; i < noc; i++) {
    unsigned int elt = get_element_from_row_with_params(in_nob, i, in_mask, elts_per_in_word, in);
    put_element_to_row_with_params(out_nob, i, out_mask, elts_per_out_word, out, elt);
  }
  return 1;
}
