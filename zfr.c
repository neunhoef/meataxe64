/*
 * $Id: zfr.c,v 1.1 2006/08/03 21:45:12 jon Exp $
 *
 * Field retract a matrix
 * This treats a matrix over GF(q**n) as being ove GF(q)
 * but with n times as many rows and columns
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "retract_matrix.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zfr";

static void fr_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file> <retraction>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  unsigned int out_prime;

  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    fr_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  out_prime = strtoul(argv[3], NULL, 0);
  memory_init(name, memory);
  endian_init();
  if (0 == retract_matrix(in, out, out_prime, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
