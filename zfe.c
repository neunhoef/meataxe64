/*
 * $Id: zfe.c,v 1.4 2004/01/04 21:22:50 jon Exp $
 *
 * Field extend a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "extend_matrix.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zfe";

static void fe_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file> <extension>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  unsigned int out_prime;

  argv = parse_line(argc, argv, &argc);
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
