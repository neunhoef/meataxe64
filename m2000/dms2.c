/*
 * Compute GF(2) sign of a matrix over Z/8Z
 * Returns 0 for is a square, 255 for isn't and 1 for error
 *
 */

#include "det.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "dms2.h"
#include "primes.h"
#include "read.h"

int dms2(const char *m, const char *name)
{
  word d, mult;
  FILE *inp;
  const header *h;
  u32 prime;
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
  if (0 == det2(m, &d, name)) {
    return 1;
  }
  mult = header_get_nor(h) / 2;
  header_free(h);
  /*
   * For 0 mod 4 1 is +, 5 is -
   * For 2 mod 2 7 is +, 3 is -
   * All other values are illegal
   */
  if (0 == (mult % 2)) {
    /* 0 mod 4 */
    if (1 != d && 5 != d) {
      fprintf(stderr, "%s: unexpected result %" W_F " for determinant, terminating\n", name, d);
      return 1;
    }
    return (1 == d) ? 0 : 255;
  } else {
    /* 2 mod 4 */
    if (7 != d && 3 != d) {
      fprintf(stderr, "%s: unexpected result %" W_F " for determinant, terminating\n", name, d);
      return 1;
    }
    return (7 == d) ? 0 : 255;
  }
}
