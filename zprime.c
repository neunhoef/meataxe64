/*
 * $Id: zprime.c,v 1.6 2004/01/04 21:22:50 jon Exp $
 *
 * Print the field order from a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "endian.h"
#include "header.h"
#include "parse.h"
#include "primes.h"
#include "read.h"

static const char *name = "zprime";

static void prime_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  FILE *inp;
  unsigned int prime;
  const header *h;

  endian_init();
  argv = parse_line(argc, argv, &argc);
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
