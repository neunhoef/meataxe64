/*
 * $Id
 *
 * Print the field characteristic from a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "endian.h"
#include "header.h"
#include "primes.h"
#include "read.h"

static const char *name = "zchar";

static void char_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  FILE *inp;
  unsigned int prime, ch;
  const header *h;

  endian_init();
  if (2 != argc) {
    char_usage();
    exit(1);
  }
  if (0 == open_and_read_binary_header(&inp, &h, argv[1], name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  header_free(h);
  assert(is_a_prime_power(prime));
  ch = prime_divisor(prime);
  if (0 == ch) {
    fprintf(stderr, "%s: %s has bad prime power %d\n", name, argv[1], prime);
    exit(1);
  }
  printf("%d\n", ch);
  fclose(inp);
  return 0;
}
