/*
 * Function to generate elements of the span of a set of rows
 *
 */

#include "span.h"
#include <stdio.h>
#include <assert.h>
#include "primes.h"

void span(u32 nor, word *vector, u32 prime,
          u32 *out_num)
{
  u32 i, j, k, l;
  assert(0 != nor);
  assert(NULL != vector);
  assert(NULL != out_num);
  assert(0 != prime);
  assert(is_a_prime_power(prime));
  i = 0;
  while (i + 1 < nor) {
    j = vector[i];
    if (0 == j) {
      /* Can safely increment this to 1 */
      /* This also gets us started from initial conditions */
      vector[i] = 1;
      *out_num = i;
      return;
    }
    /* First check if this is the highest index of a non-zero value */
    /* If so, then we don't want to increment. Instead, we move on */
    if (1 == j) {
      l = i;
      for (k = i + 1; k < nor; k++) {
        if (0 != vector[k]) {
          l = k;
          break;
        }
      }
      if (l == i) {
        /* No further non-zero entries found */
        vector[i] = 0;
        vector[i + 1] = 1;
        *out_num = i + 1;
        return;
      }
    }
    /* Can safely increment, as this isn't the leading one */
    j++;
    if (j < prime) {
      vector[i] = j;
      *out_num = i;
      return;
    }
    /* Try the next position */
    vector[i] = 0;
    i++;
  }
  /* Here we have i + 1 == nor, and vector[i] has overflowed */
  vector[i] = 0; /* Wrap around */
  *vector = 1;
  *out_num = 0;
}
