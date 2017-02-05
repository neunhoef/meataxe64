/*
 * Compute (-1)^n * determinant of a matrix mod squares
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
  word d, i, mult;
  FILE *inp;
  const header *h;
  u32 prime;
  prime_ops prime_operations;
  if (0 == open_and_read_binary_header(&inp, &h, m, name)) {
    return 1;
  }
  fclose(inp);
  prime = header_get_prime(h);
  if (1 == prime) {
    fprintf(stderr, "Det mod squares is meaningless for permutations, terminating");
    return 1;
  }
  if (header_get_noc(h) != header_get_nor(h)) {
    fprintf(stderr, "Det mod squares is meaningless for non square matrices, terminating");
    return 1;
  }
  if (0 != header_get_nor(h) % 2) {
    fprintf(stderr, "Det mod squares is meaningless for odd degree matrices, terminating");
    return 1;
  }
  if (0 == det(m, &d, name)) {
    return 1;
  }
  mult = header_get_nor(h) / 2;
  header_free(h);
  mult = (0 == mult % 2) ? 1 : prime_divisor(prime) - 1;
  /* Now see if (-1^n) * d is a square in the field */
  primes_init(prime, &prime_operations);
  d = prime_operations.mul(d, mult);
  for (i = 0; i < prime; i++) {
    if (d == prime_operations.mul(i, i)) {
      return 0;
    }
  }
  return 255;
}
