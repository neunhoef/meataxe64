/*
 * $Id: zfe.c,v 1.2 2002/06/25 10:30:12 jon Exp $
 *
 * Field extend a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "extend_matrix.h"
#include "endian.h"
#include "memory.h"

static const char *name = "zfe";

static void fe_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <extension>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  unsigned int out_prime;

  if (4 != argc) {
    fe_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  out_prime = strtoul(argv[3], NULL, 0);
  memory_init(name, 0);
  endian_init();
  if (0 == extend_matrix(in, out, out_prime, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
