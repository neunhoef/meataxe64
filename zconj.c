/*
 * $Id: zconj.c,v 1.1 2002/02/05 19:50:56 jon Exp $
 *
 * Algebriac conjugate a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "conj.h"

static const char *name = "zconj";

static void conj_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <power>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  unsigned int power;

  if (4 != argc) {
    conj_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  power = strtoul(argv[3], NULL, 0);
  memory_init(name, 0);
  endian_init();
  if (0 == conjugate(in, out, power, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
