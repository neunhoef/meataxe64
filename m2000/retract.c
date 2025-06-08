/*
 * $Id: retract.c,v 1.1 2006/08/03 21:45:12 jon Exp $
 *
 * Function to retract the field of a row
 *
 */

#include "retract.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "elements.h"
#include "primes.h"
#include "utils.h"

int retract(const word *in, word *out,
            u32 in_nob, u32 out_nob,
            u32 out_prime, u32 power,
            u32 noc)
{
  u32 i, j, elts_per_in_word, elts_per_out_word;
  word in_mask, out_mask;
  assert(NULL != in);
  assert(NULL != out);
  NOT_USED(power);
  /* TODO: fix up the row output */
  in_mask = get_mask_and_elts(in_nob, &elts_per_in_word);
  out_mask = get_mask_and_elts(out_nob, &elts_per_out_word);
  for (i = 0; i < noc; i++) {
    word elt = get_element_from_row_with_params(in_nob, i, in_mask, elts_per_in_word, in);
    u32 base = power *i;
    for (j = 0; j < power; j++) {
      put_element_to_row_with_params(out_nob, base + j, out_mask, elts_per_out_word, out, elt %out_prime);
      elt /= out_prime;
    }
  }
  return 1;
}
