/*
 * $Id: zpco.c,v 1.4 2004/01/04 21:22:50 jon Exp $
 *
 * Permutation condense
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "pco.h"
#include "utils.h"

static const char *name = "zpco";

static void pco_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <orbit_set> <perm> <condensation> <field order>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  unsigned int prime;
  const char *in1, *in2, *out;
  endian_init();
  argv = parse_line(argc, argv, &argc);
  if (5 != argc) {
    pco_usage();
    exit(1);
  }
  in1 = argv[1];
  in2 = argv[2];
  out = argv[3];
  prime = strtoul(argv[4], NULL, 0);
  memory_init(name, memory);
  endian_init();
  if (0 == pcondense(in1, in2, prime, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
