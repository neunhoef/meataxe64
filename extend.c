/*
 * $Id: extend.c,v 1.1 2002/01/22 08:40:24 jon Exp $
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
  unsigned int i;
  assert(NULL != in);
  assert(NULL != out);
  if (0 == is_a_prime(in_prime) || 0 == is_a_prime_power(out_prime) || 0 != out_prime % in_prime) {
    fprintf(stderr, "%s: bad prime %d or prime power %d, terminating\n", name, in_prime, out_prime);
    return 0;
  }
  for (i = 0; i < noc; i++) {
    unsigned int elt = get_element_from_row(in_nob, i, in);
    put_element_to_row(out_nob, i, out, elt);
  }
  return 1;
}
