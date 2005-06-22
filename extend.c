/*
 * $Id: extend.c,v 1.5 2005/06/22 21:52:53 jon Exp $
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

int extend(const word *in, word *out,
           u32 in_nob, u32 out_nob,
           u32 in_prime, u32 out_prime,
           u32 noc, const char *name)
{
  u32 i, elts_per_in_word, elts_per_out_word;
  word in_mask, out_mask;
  assert(NULL != in);
  assert(NULL != out);
  if (0 == is_a_prime(in_prime) || 0 == is_a_prime_power(out_prime) || 0 != out_prime % in_prime) {
    fprintf(stderr, "%s: bad prime %d or prime power %d, terminating\n", name, in_prime, out_prime);
    return 0;
  }
  in_mask = get_mask_and_elts(in_nob, &elts_per_in_word);
  out_mask = get_mask_and_elts(out_nob, &elts_per_out_word);
  for (i = 0; i < noc; i++) {
    word elt = get_element_from_row_with_params(in_nob, i, in_mask, elts_per_in_word, in);
    put_element_to_row_with_params(out_nob, i, out_mask, elts_per_out_word, out, elt);
  }
  return 1;
}
