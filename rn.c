/*
 * $Id: rn.c,v 1.1 2001/11/07 22:35:27 jon Exp $
 *
 * Compute the rank of a matrix
 *
 */

#include <stdio.h>
#include "clean.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "primes.h"
#include "read.h"

static const char *name = "zrn";

static void rn_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  FILE *inp;
  unsigned int prime, nod, noc, nor, nob, len;
  const header *h;
  prime_ops prime_operations;

  endian_init();
  memory_init(name, 0);
  if (2 != argc) {
    rn_usage();
    exit(1);
  }
  in = argv[1];
  inp = fopen(in, "rb");
  if (NULL == inp) {
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, in);
    exit(1);
  }
  if (0 == read_binary_header(inp, &h, in)) {
    fclose(inp);
    exit(1);
  }
  prime = header_get_prime(h);
  nob = header_get_nob(h);
  nod = header_get_nod(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  len = header_get_len(h);
  if (memory_rows(len, 1000) < nor) {
    fprintf(stderr, "%s: cannot allocate %d rows for %s, terminating\n", name, len, in);
    fclose(inp);
    exit(1);
  }
  if (0 == primes_init(prime, &prime_operations)) {
    fprintf(stderr, "%s: cannot initialise prime operations, terminating\n", name);
    fclose(inp);
    exit(1);
  }
  fclose(inp);
  memory_dispose();
  return 0;
}
