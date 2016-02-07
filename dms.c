/*
 * Compute the determinant of a matrix mod squares
 * Returns 0 for is a square, 255 for isn't and 1 for error
 *
 */

#include "det.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "dms.h"
#include "primes.h"
#include "read.h"

int dms(const char *m, const char *name)
{
  word d, i;
  FILE *inp;
  const header *h;
  u32 prime;
  prime_ops prime_operations;
  if (0 == det(m, &d, name)) {
    return 1;
  }
  /* Now see if d is a square in the field */
  if (0 == open_and_read_binary_header(&inp, &h, m, name)) {
    return 1;
  }
  prime = header_get_prime(h);
  header_free(h);
  fclose(inp);
  if (1 == prime) {
    fprintf(stderr, "Det mod squares is meaningless for permutations, terminating");
    return 1;
  }
  primes_init(prime, &prime_operations);
  for (i = 0; i < prime; i++) {
    if (d == prime_operations.mul(i, i)) {
      return 0;
    }
  }
  return 255;
}
