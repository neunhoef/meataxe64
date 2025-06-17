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
#include "parse.h"
#include "primes.h"
#include "read.h"

static const char *name = "zchar";

static void char_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  FILE *inp;
  u32 prime, ch = 1;
  const header *h;

  endian_init();
  argv = parse_line(argc, argv, &argc);
  if (2 != argc) {
    char_usage();
    exit(1);
  }
  if (0 == open_and_read_binary_header(&inp, &h, argv[1], name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  header_free(h);
  assert(1 == prime || is_a_prime_power(prime));
  if (1 != prime) {
    ch = prime_divisor(prime);
    if (0 == ch) {
      fprintf(stderr, "%s: %s has bad prime power %u\n", name, argv[1], prime);
      exit(1);
    }
  }
  printf("%u\n", ch);
  fclose(inp);
  return 0;
}
