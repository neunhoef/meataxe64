/*
 * $Id: zprime.c,v 1.3 2002/04/10 23:33:27 jon Exp $
 *
 * Print the field order from a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "endian.h"
#include "header.h"
#include "primes.h"
#include "read.h"

static const char *name = "zprime";

static void prime_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  FILE *inp;
  unsigned int prime;
  const header *h;

  endian_init();
  if (2 != argc) {
    prime_usage();
    exit(1);
  }
  if (0 == open_and_read_binary_header(&inp, &h, argv[1], name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  header_free(h);
  assert(1 == prime || is_a_prime_power(prime));
  printf("%d\n", prime);
  fclose(inp);
  return 0;
}
