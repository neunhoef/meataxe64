/*
 * $Id: zpcol.c,v 1.1 2005/06/22 21:52:54 jon Exp $
 *
 * Permutation condense wrt non trivial linear representation
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "pcol.h"
#include "utils.h"

static const char *name = "zpcol";

static void pcol_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <orbit_set> <orbit_set_lambda> <perm> <condensation>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in1, *in2, *in3, *out;
  endian_init();
  argv = parse_line(argc, argv, &argc);
  if (5 != argc) {
    pcol_usage();
    exit(1);
  }
  in1 = argv[1];
  in2 = argv[2];
  in3 = argv[3];
  out = argv[4];
  memory_init(name, memory);
  endian_init();
  if (0 == pcondense_lambda(in1, in2, in3, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
